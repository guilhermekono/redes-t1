#!/bin/bash

cat roteador.config | while read line; do
  r_id=$(echo "$line" | awk '{print $1}')
  xterm -hold -e "./main $r_id" &
done
