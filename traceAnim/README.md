# Trace animation

This is a remake of the famous Hacknet trace animation. Done with gtk2.0 and C. There are some commented configuration values in the source file.
Build it with `make build`.

You can configure the colors, fullscreen window geometry, timings, and line heights.
There is also a feature that allows you to run an arbitrary command using `int system(const char *command);`, when the animation is finished. (would make a nice shutdown animation, for example)

## Screenshots

### Original

![original animation screenshot from youtube.com](https://i.ytimg.com/vi/XdPmJxJHf98/maxresdefault.jpg)
### Project HacknetGTK (excuse the recording quality)

![recorded webm of traceAnim](./screenshots/traceanim.mp4)
