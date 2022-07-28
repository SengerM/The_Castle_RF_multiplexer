# The Castle RF multiplexer Python interface

A simple and easy interface to control *The Castle*, an RF multiplexer I made for the lab to automate the measurement of LGAD devices.

# Installation

```
pip install -e PATH_TO_THE_DIRECTORY_WHERE_THIS_README_FILE_IS
```

# Usage

```python
from The_Castle_RF_multiplexer import The_Castle

for cmd in {'IDN?','VERSION?','inexistent_command'}:
	print(cmd)
	print(The_Castle.query(cmd))

for k in [1,2,3,4,5,6,7,8]:
	print(f'Connecting channel {k}...')
	The_Castle.connect_channel(k)
```
