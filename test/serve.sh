#!/bin/bash -eu
npm run build:vue
npm run build:ci &
npm run serve:no-port -- -p $serport