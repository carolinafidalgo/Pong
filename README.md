# Pong

In the Relay-Pong, users take turns playing the pong game individually on their client,
until the server releases their ball and the control goes to other user.
When a client is not controlling the ball, his paddle will not move, but the ball position
(controlled by another client) is updated on the screen.
When a client starts controlling the ball, this client can start to move the paddle and hit the
ball, and move the ball. Even if the paddle does not move, the ball should move one place
every second.
Any user can press the Q key to quit at any time.
This version still uses internet datagram sockets.


In the Super-Pong, all players (connected to the server in multiple clients) can move
their paddles and try to hit the ball simultaneous. This is accomplished by reading the paddle
movement on each client, and sending such movements to the server, which updates the ball
(if hit by a paddle) and sends back to all the clients the new board state.
The clients send every paddle movement to the server, without receiving any explicit reply to
this message.
The Super-Pong uses Internet stream sockets so the Connect and Disconnect
messages can be omitted and replaced by the socket connect/accept and close
