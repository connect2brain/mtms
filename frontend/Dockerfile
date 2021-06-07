FROM node:lts-alpine

WORKDIR /app

COPY frontend/package*.json ./

RUN npm install

# The environment variables are not passed onto the Vue service automatically,
# instead they are read by Vue from the directory in which the Vue is run. Thus,
# create a copy inside the container.
COPY .env ./

COPY frontend/* ./

COPY frontend/public ./

COPY frontend/tests ./

CMD [ "npm", "run", "serve" ]
