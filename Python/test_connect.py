import socket
s = socket.socket()
s.settimeout(5)
try:
    s.connect(('127.0.0.1', 55557))
    print('Connected successfully!')
    s.close()
except Exception as e:
    print(f'Connection failed: {e}')
