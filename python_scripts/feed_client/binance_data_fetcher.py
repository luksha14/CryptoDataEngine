import socket
import datetime
import asyncio
from binance import AsyncClient, BinanceSocketManager

SERVER_IP = "127.0.0.1"
SERVER_PORT = 12345 
SYMBOL = 'btcusdt'  

# --- API KEYS ---
# API keys are not required for public trade streams.
API_KEY = '' 
API_SECRET = ''

class LiveTradeClient:
    def __init__(self, ip, port, symbol):
        self.ip = ip
        self.port = port
        self.symbol = symbol
        self.tcp_socket = None
        self.client = None
        self.trade_counter = 0 

    async def connect_to_binance(self):
        """Initializes Binance connection."""
        print(f"[{datetime.datetime.now().strftime('%H:%M:%S')}] Initializing Binance Async Client...")
        self.client = await AsyncClient.create(api_key=API_KEY, api_secret=API_SECRET)
        self.bsm = BinanceSocketManager(self.client)

    def connect_to_engine(self):
        """Connects to the C++ Data Engine via TCP."""
        try:
            print(f"[{datetime.datetime.now().strftime('%H:%M:%S')}] Connecting to C++ Engine at {self.ip}:{self.port}...")
            self.tcp_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.tcp_socket.connect((self.ip, self.port))
            print("Successfully connected to C++ Engine via TCP.")
            return True
        except ConnectionRefusedError:
            print("ERROR: Connection refused. Is the C++ Engine running?")
            return False
        except Exception as e:
            print(f"An error occurred during TCP connection: {e}")
            return False

    async def start_trade_feed(self):
        """Subscribes to the live trade feed (@trade) and sends data, including Trade ID, to the C++ Engine."""
        
        trade_socket = self.bsm.trade_socket(symbol=self.symbol) 

        async with trade_socket as stream:
            while True:
                msg = await stream.recv()
                
                if 'e' in msg and msg['e'] == 'trade':
                    trade_time_ms = msg['E'] 
                    trade_id = msg['t']      # Jedinstveni Trade ID
                    price = float(msg['p'])
                    quantity = float(msg['q'])
                    
                    self.trade_counter += 1
                    
                    # Format: timestamp_ms, symbol, trade_id, open, high, low, close, volume
                    # All OHLC are set to trade price for the trade stream
                    data_string = (
                        f"{trade_time_ms},{self.symbol.upper()},{trade_id},{price:.8f},{price:.8f},"
                        f"{price:.8f},{price:.8f},{quantity:.8f}\n" 
                    )
                    
                    try:
                        self.tcp_socket.sendall(data_string.encode('utf-8'))
                        
                        if self.trade_counter % 50 == 0:
                            print(f"[{datetime.datetime.fromtimestamp(trade_time_ms/1000).strftime('%H:%M:%S.%f')[:-3]}] Sent {self.trade_counter} trades. Last Price: {price:.2f} (ID: {trade_id})")

                    except socket.error as e:
                        print(f"Socket send error: {e}. Attempting to close and reconnect...")
                        self.tcp_socket.close()
                        if not self.connect_to_engine():
                            print("Reconnection failed. Exiting trade feed.")
                            break 

    async def run(self):
        if not self.connect_to_engine():
            return

        await self.connect_to_binance()
        
        try:
            await self.start_trade_feed()
        except Exception as e:
            print(f"An unexpected error occurred in the feed: {e}")
        finally:
            if self.client:
                await self.client.close_connection()
            if self.tcp_socket:
                self.tcp_socket.close()
            print("Live Trade Client shutting down...")


if __name__ == "__main__":
    client = LiveTradeClient(
        ip=SERVER_IP, 
        port=SERVER_PORT, 
        symbol=SYMBOL
    )
    
    try:
        asyncio.run(client.run())
    except KeyboardInterrupt:
        print("Live Trade Client interrupted by user (Ctrl+C). Shutting down...")