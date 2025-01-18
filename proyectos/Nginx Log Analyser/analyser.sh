#!/bin/bash

LOG_FILE="nginx_access.log"

if [[ ! -f "$LOG_FILE" ]]; then
    echo "Log file $LOG_FILE not found!"
    exit 1
fi

echo "Top 5 IPs con mas peticiones:"
awk '{print $1}' "$LOG_FILE" | sort | uniq -c | sort -nr | head -5

echo "\nTop 5 de paths solicitados:"
grep -oP '"GET \K[^ ]+' "$LOG_FILE" | sort | uniq -c | sort -nr | head -5

echo "\nTop 5 status codes:"
awk '{print $9}' "$LOG_FILE" | sort | uniq -c | sort -nr | head -5

echo "\nTop 5 user agents:"
awk -F\" '{print $6}' "$LOG_FILE" | sort | uniq -c | sort -nr | head -5
