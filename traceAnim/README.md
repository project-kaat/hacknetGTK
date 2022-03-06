# Trace animation

This is a remake of the famous Hacknet trace animation. Done with gtk2.0 and C. There are some commented configuration values in the source file.
Build it with `make build`.

You can configure the colors, fullscreen window geometry, timings, and line heights.
There is also a feature that allows you to run an arbitrary command using `int system(const char *command);`, when the animation is finished. (would make a nice shutdown animation, for example)

## Screenshots

### Original

![original animation screenshot from youtube.com](https://www.google.com/url?sa=i&url=https%3A%2F%2Fwww.youtube.com%2Fwatch%3Fv%3DXdPmJxJHf98&psig=AOvVaw25bHXknhCOj-TMPrA9FTU6&ust=1646395308305000&source=images&cd=vfe&ved=0CAgQjRxqFwoTCMCeus7yqfYCFQAAAAAdAAAAABAE)
### Project HacknetGTK (excuse the webm quality)

![recorded webm of traceAnim](./screenshots/traceanim.webm)
