#!/bin/bash -eu
npm run build:vue
npm run build:ci &
port="${serport:-12581}"
npm run serve ./ -p $port