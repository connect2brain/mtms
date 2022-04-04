FROM node:12-alpine

WORKDIR /app

COPY frontend/package*.json ./

RUN npm install

COPY frontend/.eslintrc.js ./

COPY frontend/.browserslistrc ./

COPY frontend/*.config.js ./

COPY frontend/public public

COPY frontend/tests tests

# The environment variables are not passed onto the Vue service automatically,
# instead they are read by Vue from the directory in which the Vue is run. Thus,
# create a copy inside the container.
COPY .env ./

CMD [ "npm", "run", "serve" ]
