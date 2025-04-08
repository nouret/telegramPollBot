# telegramPollBot

Bot which can wait command and post poll in telegram chat

### Requirements

- meson
- curl libruary
- json libruary

With apt:
`apt install -y meson libcurl4-openssl-dev rapidjson-dev`

### Building

First, clone the repository:

```bash
git clone https://github.com/nouret/telegramPollBot
cd telegramPollBot
```

Then, to build the program, run this in the source directory:

```bash
meson build
ninja -C build
```
