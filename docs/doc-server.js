const static = require('node-static')
const http = require('http')

const file = new(static.Server)(`${__dirname}/build/html`)

const serverPort = 5000
const server = http.createServer((req, res) => {
    file.serve(req, res)
})

server.listen(serverPort)
console.log(`NodeJS web server at port ${serverPort} is running..`)