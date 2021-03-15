FROM node:lts-alpine

WORKDIR /app

COPY frontend/package*.json ./

RUN npm install

COPY frontend/* ./

COPY frontend/public ./

COPY frontend/tests ./

CMD [ "npm", "run", "serve" ]
