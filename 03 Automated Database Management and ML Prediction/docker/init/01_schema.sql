-- Enable TimescaleDB extension when available; degrade gracefully to plain PostgreSQL.
DO $$
BEGIN
    CREATE EXTENSION IF NOT EXISTS timescaledb;
EXCEPTION WHEN OTHERS THEN
    RAISE NOTICE 'TimescaleDB extension not available, using plain PostgreSQL tables: %', SQLERRM;
END $$;

-- Ticks table: raw trades/quotes
CREATE TABLE IF NOT EXISTS ticks (
    time TIMESTAMPTZ NOT NULL,
    symbol TEXT NOT NULL,
    price DOUBLE PRECISION NOT NULL,
    size DOUBLE PRECISION NOT NULL,
    side SMALLINT, -- -1 sell, 0 unknown, 1 buy
    source TEXT,
    received_at TIMESTAMPTZ DEFAULT NOW()
);

DO $$
BEGIN
    PERFORM create_hypertable('ticks', 'time', if_not_exists => TRUE, chunk_time_interval => INTERVAL '1 day');
EXCEPTION WHEN OTHERS THEN
    RAISE NOTICE 'Could not create hypertable ticks: %', SQLERRM;
END $$;
CREATE INDEX IF NOT EXISTS idx_ticks_symbol_time ON ticks (symbol, time DESC);

-- Per-ticker tick tables. The C++ ETL writer creates these on the fly;
-- this helper procedure pre-creates them for the default top-ten universe.
CREATE OR REPLACE FUNCTION create_tick_table(p_symbol TEXT)
RETURNS TEXT AS $$
DECLARE
    table_name TEXT := 'ticks_' || lower(regexp_replace(p_symbol, '[^a-zA-Z0-9]', '_', 'g'));
BEGIN
    EXECUTE format(
        'CREATE TABLE IF NOT EXISTS %I (
            time TIMESTAMPTZ NOT NULL,
            symbol TEXT NOT NULL,
            price DOUBLE PRECISION NOT NULL,
            size DOUBLE PRECISION NOT NULL,
            side SMALLINT,
            source TEXT,
            received_at TIMESTAMPTZ DEFAULT NOW()
        )',
        table_name
    );
    EXECUTE format(
        'CREATE INDEX IF NOT EXISTS idx_%s_symbol_time ON %I (symbol, time DESC)',
        table_name, table_name
    );
    RETURN table_name;
END;
$$ LANGUAGE plpgsql;

DO $$
DECLARE
    sym TEXT;
    default_symbols TEXT[] := ARRAY['AAPL','MSFT','NVDA','AMZN','GOOGL','META','TSLA','AVGO','BRK_B','JPM'];
BEGIN
    FOREACH sym IN ARRAY default_symbols LOOP
        PERFORM create_tick_table(sym);
    END LOOP;
END $$;

-- OHLCV bars
CREATE TABLE IF NOT EXISTS bars_1m (
    time TIMESTAMPTZ NOT NULL,
    symbol TEXT NOT NULL,
    open DOUBLE PRECISION NOT NULL,
    high DOUBLE PRECISION NOT NULL,
    low DOUBLE PRECISION NOT NULL,
    close DOUBLE PRECISION NOT NULL,
    volume DOUBLE PRECISION NOT NULL,
    vwap DOUBLE PRECISION,
    trades INTEGER
);

DO $$
BEGIN
    PERFORM create_hypertable('bars_1m', 'time', if_not_exists => TRUE, chunk_time_interval => INTERVAL '7 days');
EXCEPTION WHEN OTHERS THEN
    RAISE NOTICE 'Could not create hypertable bars_1m: %', SQLERRM;
END $$;
CREATE INDEX IF NOT EXISTS idx_bars_symbol_time ON bars_1m (symbol, time DESC);

-- Engineered features per bar
CREATE TABLE IF NOT EXISTS features (
    time TIMESTAMPTZ NOT NULL,
    symbol TEXT NOT NULL,
    returns DOUBLE PRECISION,
    log_returns DOUBLE PRECISION,
    realized_vol DOUBLE PRECISION,
    sma_5 DOUBLE PRECISION,
    sma_20 DOUBLE PRECISION,
    ema_12 DOUBLE PRECISION,
    rsi_14 DOUBLE PRECISION,
    macd DOUBLE PRECISION,
    macd_signal DOUBLE PRECISION,
    bb_position DOUBLE PRECISION,
    momentum_10 DOUBLE PRECISION,
    volume_sma_10 DOUBLE PRECISION,
    target_direction SMALLINT,
    target_return DOUBLE PRECISION
);

DO $$
BEGIN
    PERFORM create_hypertable('features', 'time', if_not_exists => TRUE, chunk_time_interval => INTERVAL '7 days');
EXCEPTION WHEN OTHERS THEN
    RAISE NOTICE 'Could not create hypertable features: %', SQLERRM;
END $$;
CREATE INDEX IF NOT EXISTS idx_features_symbol_time ON features (symbol, time DESC);

-- Model predictions / signals
CREATE TABLE IF NOT EXISTS signals (
    time TIMESTAMPTZ NOT NULL,
    symbol TEXT NOT NULL,
    model_name TEXT NOT NULL,
    prediction SMALLINT NOT NULL, -- -1, 0, 1
    probability DOUBLE PRECISION,
    feature_vector JSONB,
    metadata JSONB
);

DO $$
BEGIN
    PERFORM create_hypertable('signals', 'time', if_not_exists => TRUE, chunk_time_interval => INTERVAL '7 days');
EXCEPTION WHEN OTHERS THEN
    RAISE NOTICE 'Could not create hypertable signals: %', SQLERRM;
END $$;
CREATE INDEX IF NOT EXISTS idx_signals_symbol_time ON signals (symbol, time DESC);

-- Model registry
CREATE TABLE IF NOT EXISTS model_registry (
    model_name TEXT PRIMARY KEY,
    version TEXT NOT NULL,
    trained_at TIMESTAMPTZ DEFAULT NOW(),
    metrics JSONB NOT NULL,
    onnx_path TEXT NOT NULL,
    feature_columns TEXT[] NOT NULL,
    hyperparameters JSONB
);
