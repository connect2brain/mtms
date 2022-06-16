# Taken from https://mherman.org/blog/dockerizing-a-react-app/
# pull official base image
FROM node:16.13.2-alpine

# set working directory
WORKDIR /app

# add `/app/node_modules/.bin` to $PATH
ENV PATH /app/node_modules/.bin:$PATH

# install app dependencies
COPY package.json ./
COPY yarn.lock ./
# RUN npm install --global yarn
# RUN npm install --silent
# RUN npm install react-scripts@5.0.0 -g --silent
RUN yarn install --silent
RUN yarn global add react-scripts@5.0.0 --silent

# Fixes .cache EACCESS error. Taken from https://stackoverflow.com/questions/67087735/eacces-permission-denied-mkdir-usr-app-node-modules-cache-how-can-i-creat
RUN mkdir -p node_modules/.cache && chmod -R 777 node_modules/.cache

# add app
COPY . ./

# start app
CMD ["yarn", "start"]