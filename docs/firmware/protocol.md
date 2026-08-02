# Firmware protocol

The controller exposes its versioned motion protocol over USB serial. `MOTOR HOME` is
rejected on the owned profile because no physical home switches are installed. Motion
commands remain bounded by the profile's cable-protection soft limits.
