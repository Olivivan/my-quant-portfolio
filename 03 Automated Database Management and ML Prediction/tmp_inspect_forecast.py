import sys
from pathlib import Path
sys.path.insert(0, 'src/python')
from visualization.forecast import generate_forecast
from visualization.plot_predictions import load_raw_prices, load_signals
from common.config import Config
import pandas as pd

cfg = Config('config/pipeline.json')
prices = load_raw_prices(cfg)
signals = load_signals(cfg, cfg.inference_cfg['model_name'])

last_date = prices[prices['symbol'] == 'AAPL']['time'].max()
last_price = prices[(prices['symbol'] == 'AAPL') & (prices['time'] == last_date)]['close'].values[0]

fc = generate_forecast(
    symbol='AAPL',
    last_price=last_price,
    last_date=last_date,
    end_date=pd.Timestamp('2030-12-31'),
    signals=signals,
    prices=prices,
)
print(fc.head())
print(fc.tail())
print('forecast rows:', len(fc))
print('AAPL last price:', last_price, 'last date:', last_date)
