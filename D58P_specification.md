# D58P Specification

## Requests

```
/D58P /RequestName
<data1>
<data2>
...
```

Any data sent over the TCP socket of the above form will be interpreted as a D58P request. 

First the /D58P, then followed by forward-slash with the type of request.

Then on each following line separated by newline characters are data required for the particular D58P request.

__Examples:__

```
/D58P /User
<user>
<pass>
```
Represents a D58P User request. When the server receives this it will try to authenticate the user using specified username and password.


```
D58P /Message
<user>
<password>
<target user>
<message>
```
Represents a D58P Message request. When the server receives this it will try to send `message` from `user` to `target user`. Uses `password` to authenticate `user`.


## Responses

```
/D58P \ResponseName
<code>
<data1>
<data2>
...
```
Above represents a D58P response. 

First the /D58P, then followed by back-slash (instead of forward slash) with the type of response.

On the second line is the response code. D58P response codes are analogous to HTTP response codes. So `code=200` indicates OK, `code=400` indicates BAD REQUEST.

Then on each following line separated by newline characters are data required for the particular D58P response.

__Examples:__

```
/D58P \User
<code>
```
Represents a D58P User response. `code` refers to the D58P code for the response. `code=200` indicates a successful login. `code=201` indicates successful register. `code=401` indicates unauthorized.

```
/D58P \Message
<code>
```
Represents a D58P Message response.
