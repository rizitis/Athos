#!/bin/bash
curl -s https://www.phoronix.com/news | grep '<div class="popular-list">' -A 2 | sed -n 's/.*<a href="\([^"]*\)">\(.*\)<\/a>.*/\2 - https:\/\/www.phoronix.com\1/p' > /tmp/news.text

# Define an array of colors using ANSI escape codes
COLORS=(
  '\033[0;31m'
  '\033[0;32m'
  '\033[0;34m'
  '\033[0;33m'
  '\033[0;35m'
  '\033[0;36m'
)

NC='\033[0m'

echo "-- NEWS --"
line_number=0
while IFS= read -r line; do
  color=${COLORS[line_number % ${#COLORS[@]}]}
  echo -e "${color}${line}${NC}"
  ((line_number++))
done < /tmp/news.text
echo ""
echo "-- Plasma --"
cat /tmp/news.text | grep KDE
cat /tmp/news.text | grep Plasma
cat /tmp/news.text | grep Qt6
