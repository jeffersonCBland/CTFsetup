
#include <stdio.h>
#include <stdlib.h>
#include <hiredis/hiredis.h>
#include <postgresql/libpq-fe.h> // Header for Postgres

int main() {
    // 1. CONNECT TO REDIS
    redisContext *redis = redisConnect("127.0.0.1", 6379);
    if (redis == NULL || redis->err) {
        fprintf(stderr, "Redis connection error: %s\n", redis ? redis->errstr : "Allocation failed");
        return 1;
    }

    // 2. CONNECT TO POSTGRES
    // host is reis for redis container
    PGconn *pg = PQconnectdb("dbname=ctf_db user=ctf_user password=cpsec host=redis");
    if (PQstatus(pg) != CONNECTION_OK) {
        fprintf(stderr, "Postgres connection failed: %s\n", PQerrorMessage(pg));
        redisFree(redis);
        PQfinish(pg);
        return 1;
    }

    // 3. READ FROM REDIS
    redisReply *reply = redisCommand(redis, "GET %s", "team:123:bio");
    if (reply == NULL || reply->type != REDIS_REPLY_STRING) {
        fprintf(stderr, "Failed to read string from Redis or key doesn't exist\n");
        if (reply) freeReplyObject(reply);
        redisFree(redis);
        PQfinish(pg);
        return 1;
    }

    // Extract the string value from Redis response
    const char *redis_val = reply->str; 
    int user_id = 123;

    // 4. WRITE TO POSTGRES (Parameterized Query Syntax)
    const char *query = "INSERT INTO users (id, bio) VALUES ($1, $2) ON CONFLICT (id) DO UPDATE SET bio = $2;";
    
    // Convert int to string for PG parameter binding
    char id_str[12];
    sprintf(id_str, "%d", user_id);

    // Array of pointer values for parameters
    const char *paramValues[2] = { id_str, redis_val };

    // Execute the statement safely
    PGresult *res = PQexecParams(
        pg,
        query,
        2,             // Number of parameters
        NULL,          // Param types (OIDs, let PG infer it)
        paramValues,   // Param values array
        NULL,          // Param lengths (not needed for text strings)
        NULL,          // Param formats (0 = text, 1 = binary)
        0              // Result format (0 = text)
    );

    // Check query execution status
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Postgres Insert Failed: %s\n", PQerrorMessage(pg));
    } else {
        printf("Successfully copied data from Redis to Postgres!\n");
    }

    // 5. CLEANUP IN REVERSE ORDER
    PQclear(res);
    freeReplyObject(reply);
    redisFree(redis);
    PQfinish(pg);

    return 0;
}

