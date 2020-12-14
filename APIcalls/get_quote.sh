#!/bin/bash

full_quote=$(curl -s http://api.quotable.io/random)
content=$(echo $full_quote | jq ".content")
author=$(echo $full_quote | jq ".author")
author=$(sed -e 's/^"//' -e 's/"$//' <<<"${author}")
echo "${content} -${author}"

