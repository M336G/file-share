## file-share
A simple webserver built in C++ using [cpp-httplib](https://github.com/yhirose/cpp-httplib) to share files with others!

## Usage
Once you've got your webserver running, simply add files to the directory you've set in `STORAGE_DIRECTORY` and use `GET /<file_name>` to access any of them!

You may also get a list of every file that's publicly accessible with `GET /files`, though if you'd wish to disable this, set `DISABLE_INDEX` to `true` or `1`.

## Configuration
See the notes in [.env.example](https://github.com/M336G/file-share/blob/main/.env.example) to configure your webserver.

## Contributing
Feel free to open pull requests if you wish to contribute to the project!

## License
This project is licensed under the [GNU Affero General Public License v3.0](https://github.com/M336G/file-share/blob/main/LICENSE).
