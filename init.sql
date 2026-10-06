CREATE TABLE teams (
	id SERIAL PRIMARY KEY,
	name VARCHAR(100),
	score INT DEFAULT 0,
	members SERIAL,
	solved SERIAL
);


CREATE OR REPLACE FUNCTION notify_redis_sync()
RETURNS TRIGGER AS $$

DECLARE 
payload TEXT;

BEGIN 
	payload := row_to_json(NEW)::text;
	PERFORM pg_notify('data_changes',payload)
	RETURN NEW;
END;
$$ LANGUAGE plpgsql;


CREATE TRIGGER team_redis_sync_trigger
AFTER INSERT OR UPDATE ON users
FOR EACH ROW EXECUTE FUNCTION notify_redis_sync();
