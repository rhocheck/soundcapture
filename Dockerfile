FROM node:12-alpine AS build
RUN apk --update add --no-cache python3 make g++

RUN adduser root audio

WORKDIR /usr/src/app

COPY package*.json ./
RUN npm install

COPY . .

RUN npm install -g node-gyp
RUN node-gyp configure
RUN node-gyp build

FROM node:12-alpine
RUN apk --update add --no-cache alsa-utils alsa-utils-doc alsa-lib alsaconf 
WORKDIR /usr/src/app
COPY --from=build /usr/src/app .

CMD [ "npm", "start" ]