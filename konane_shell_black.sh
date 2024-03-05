#!/bin/bash
WINS=0
GAMES=0
for i in {1..50}
do
  let GAMES++
  output=$(./drivercheck.pl konane.exe konanerandomplayer)
  if [[ "$output" == *"You won"* ]]; then
    let WINS++
  else
    printf "%s\n\n Game %s seems to have lost\n\n" "${output:(-2000)}" "$GAMES"
  fi
done

printf "Won %s out of %s times as black pieces\n" "$WINS" "$GAMES"
