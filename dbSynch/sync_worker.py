import psycopg2
import redis
import json


r = redis.Redis(host='redis', port = 6379, decode_responses=True)


pg_conn = psycopg2.connect("dbname=ctf_db user=ctf_user password=cpsec host=postgres")
pg_conn.set_isolation_level(psycopg2.extensions.ISOLATION_LEVEL_AUTOCOMMIT)

cursor = pg_conn.cursor()
cursor.execute("LISTEN data_changes;")


print("sync worker container successfully up and running")

while True:
    pg_conn.select([cursor])
    pg_conn.poll()
    while pg_conn.notifies:
        notify = pg_conn.notifies.pop(0)
        data = json.loads(notify.payload)
        redis_key = f"user:{data['id']}"
        r.set(redis_key, notify.payload)
        print(f"Sybced {redis_key} to Redis")
        
