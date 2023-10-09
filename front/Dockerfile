# Taken from https://mherman.org/blog/dockerizing-a-react-app/

# Pull official base image.
FROM node:18.18.0-alpine

# Set the working directory.
WORKDIR /app

# Add `/app/node_modules/.bin` to $PATH.
ENV PATH /app/node_modules/.bin:$PATH

# Set the Cyclone DDS configuration
COPY config/cyclonedds.xml /config/
ENV CYCLONEDDS_URI=file:///config/cyclonedds.xml

# Install app dependencies.
COPY /front/package.json /front/yarn.lock ./
RUN yarn install
RUN yarn global add react-scripts@5.0.0

# Fix .cache EACCESS error. Taken from:
#
# https://stackoverflow.com/questions/67087735/eacces-permission-denied-mkdir-usr-app-node-modules-cache-how-can-i-creat
RUN mkdir -p node_modules/.cache && chmod -R 777 node_modules/.cache

# Add app.
COPY /front/ /app

# Start app.
CMD ["yarn", "start"]
