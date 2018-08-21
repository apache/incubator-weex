#!/usr/bin/env bash
npm run build:vue
npm run build:ci &
port="${serport:-12581}"
npm run serve:no-port -- -p $port