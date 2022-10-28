#! /usr/bin/env node
var http = require('http'),
    fs = require('fs'),
    util = require('util'),
    url = require('url');

const PORT = 8080;
const redirectTimes = 10;    
let times = 0;

var server = http.createServer(function(req, res) {
    console.log(url.parse(req.url).pathname)
    redirectToTimes(req, res);
});

function redirectToTimes(req, res) {
    times++;
    let myUrl = url.parse(req.url).pathname;
    if (times <= redirectTimes && (myUrl === '/' || myUrl.includes('redirectPage'))) {
        console.log(`redirectPage${times}.html`);
        if (times < redirectTimes) {
            res.writeHead(302, {
              'Location': `redirectPage${times}.html`
            });
            res.end();
        } else {
            res.writeHead(302, {
              'Location': 'redirectedPage.html'
            });
            res.end();
        }
    } else {
        res.writeHead(200, {'content-type': 'text/html'});
        var index = fs.readFileSync('./redirectedPage.html');
        res.end(index);
        times = 0;
    }
}

console.log(`Server listening on PORT: ${PORT}`);
server.listen(PORT);
