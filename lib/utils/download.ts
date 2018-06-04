/// <reference types="request" />

namespace trusted.utils {
    const request = require("request");
    const fs = require("fs");

    /**
     * Download file
     *
     * @param {string} url Url to remote file
     * @param {string} path Path for save in local system
     * @param {Function} done callback function
     */
    export function download(url: string, path: string, done: (err: Error, url?: string, path?: string) => void): void {
        const sendReq: any = request.get(url);

        sendReq.on("response", (response) => {
            switch (response.statusCode) {
                case 200:
                    const stream = fs.createWriteStream(path);

                    response.on("data", (chunk) => {
                        stream.write(chunk);
                    }).on("end", () => {
                        stream.on("close", () => {
                            done(null, url, path);
                        });
                        stream.end();
                    });

                    break;
                default:
                    done(new Error("Server responded with status code" + response.statusCode));
            }
        });

        sendReq.on("error", (err) => {
            if (fs.existsSync(path)) {
                fs.unlink(path);
            }

            done(err);
        });
    }
}
