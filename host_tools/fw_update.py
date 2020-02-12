from .xbvc_py import *
import serial
from serial.tools import list_ports
import ihextools as ihex
import optparse
import sys


class XBVCSerialEP(XBVCEdgePoint):

    def __init__(self, serial_port):
        super(XBVCSerialEP, self).__init__()
        self._port = serial_port
        self._ser = None

    def connect(self):
        self._ser = serial.Serial(self._port, 115200, timeout=0)

    def disconnect(self):
        try:
            self._ser.close()
        except:
            pass

    def _read(self, length):
        res = self._ser.read(length)

        if len(res):
            return [ord(x) for x in res]
        else:
            return None

    def _write(self, data):
        st_data = ''.join([chr(x) for x in data])
        self._ser.write(st_data)


def progress_bar(percent_full, width=50):
    """
    Prints a progress bar in the form [*****        ] xx%
    """
    numstars = int(width * (percent_full / 100.))
    starstr = numstars * '*'
    starstr += (width - len(starstr)) * ' '
    progress_str = '\r[' + starstr + '] {}%'.format(int(percent_full))
    sys.stdout.write(progress_str)
    sys.stdout.flush()


class FwUpdater(object):

    def __init__(self, **kwargs):
        self._progress_cb = None
        self._logger = kwargs.get('logger', None)
        self._log('FWUpdater 0.3.3a')

    def _log(self, msg):
        if self._logger:
            self._logger.info("FwUpdater: {}".format(msg))
        else:
            print(msg)

    def _get_version(self, port):
        # Create our edgepoint
        try:
            ep = XBVCSerialEP(port)
            ep.timeout_ms = 1000
            ep.connect()
            ep.start()

            ver = get_version_command()

            ep.flush()

            res = ep.send(ver, GET_VERSION_RESPONSE_ID)
        except:
            res = None
        finally:
            ep.stop()
            ep.disconnect()

        return res

    def _send_ping(self, port):
        # Create our edgepoint
        try:
            ep = XBVCSerialEP(port)
            ep.timeout_ms = 1000
            ep.connect()
            ep.start()

            ping = ping_command()

            ep.flush()

            res = ep.send(ping, PING_RESPONSE_ID)
        except:
            res = None
        finally:
            ep.stop()
            ep.disconnect()

        return res

    def register_progress_callback(self, cb):
        self._progress_cb = cb

    def scan_for_device(self):
        ports = [x[0] for x in list_ports.comports()]
        retport = None
        self._log("Searching for an active bootloader")

        for p in ports:
            try:
                rsp = self._send_ping(p)
                if not rsp == None:
                    ver = self._get_version(p)
                    if not ver:
                        version = '<=0.1.2'
                    else:
                        version = '.'.join(
                            [str(x) for x in
                             (ver.major, ver.minor, ver.bugfix)])
                    self._log("Found bootloader ver: {} on: {}".format(version, p))
                    retport = p
                    break
                else:
                    raise IOError()
            except IOError:
                pass

        return retport

    def _init_xbvc(self):
        # Create our edgepoint
        self.ep = XBVCSerialEP(self.port)

        # set the timeout to 1 second
        # (since we're erasing, this can take some time)
        self.ep.timeout_ms = 1000
        self.ep.connect()
        self.ep.start()

    def _load_words_from_ihex(self):
        # Load and process the ihex file
        ih = ihex.iHex()
        ih.load_ihex(self.filepath)

        # Get a list of the words in the ihex file (and their offsets)
        wl = ih.get_u32_list()

        # Now, create 64 word chunks from this list of words
        wordchunks = [x for x in ihex.chunks(wl, 64)]

        return wordchunks

    def update_firmware(self, filepath, port, verbose=False):
        self.filepath = filepath
        self.port = port

        # initialize the xbvc edgepoint
        self._init_xbvc()

        wordchunks = self._load_words_from_ihex()

        self._log("Updating firmware...")

        # get the total number of chunks (we'll use this for the progress bar)
        num_chunks = len(wordchunks)
        chunks_remaining = num_chunks

        # Take this list of chunks and turn each chunk into a flash_command
        for chunk in wordchunks:
            fcmd = flash_command()

            # set the starting offset of this command to the offset of the
            # first word in the chunk
            fcmd.offset = chunk[0][0]

            # Set the data_length to the number of words in this chunk
            fcmd.data_len = len(chunk)

            # Now move the data words in this chunk to the data section of
            # the flash command
            chunkdata = [x[1] for x in chunk]
            for i in range(len(chunkdata)):
                fcmd.data[i] = chunkdata[i]

            # if we're in verbose mode, print everything we see on the
            # wire, otherwise, just keep a progress bar rolling
            if verbose:
                self._log(fcmd)
                self._log(len(fcmd.encode()))
            else:
                percent_complete = 100 - ((chunks_remaining / float(num_chunks)) * 100)
                if self._progress_cb:
                    self._progress_cb(percent_complete)

            # send the command and wait for a response
            res = self.ep.send(fcmd, FLASH_RESPONSE_ID)

            if res == None:
                # Retry one time
                if verbose:
                    self._log("Retrying")
                self.ep.flush()
                res = self.ep.send(fcmd, FLASH_RESPONSE_ID)
                if res == None:
                    raise Exception("Error flashing device, no response!")

                if verbose:
                    self._log(res)
            elif res.error != 0:
                raise Exception("Error flasing device, got error:{}".format(res.error))
            else:
                chunks_remaining -= 1
                if verbose:
                    self._log(res)

        if not verbose and self._progress_cb:
            self._progress_cb(100)
            self._log("\n")
        msg = verify_command()

        # wait for a verify response
        self._log("Verifying firmware")
        rsp = self.ep.send(msg, VERIFY_RESPONSE_ID)

        if verbose:
            self._log(rsp)

        if rsp.error == 0:
            self._log("Firmware was valid")
            msg = run_command()
            res = self.ep.send(msg, RUN_RESPONSE_ID)
            if res.error == 0:
                self._log("Starting firmware")
            elif res == None:
                raise Exception("Error: Bootloader did not respond to run command")
        else:
            raise Exception("Error validating flash on device. Updgrade failed")

        self.ep.stop()
        self.ep.disconnect()


def main():
    parser = optparse.OptionParser()
    parser.add_option('-f', '--filename',
                      dest="input_filename",
                      help="Filename of .ihex file to process")

    parser.add_option('-p', '--port',
                      dest="ser_port",
                      help="(optional)path to serial port we're talking on ex: /dev/ttyACM0")

    parser.add_option('-v', '--verbose',
                      dest="verbose",
                      help="turn on verbose debugging")

    input("Place your device into bootloader mode and then press any key")

    options, remainder = parser.parse_args()
    fu = FwUpdater()

    # register a text mode progress bar
    fu.register_progress_callback(progress_bar)

    if not options.input_filename:
        print("You must provide an input filename")
        parser.print_help()
        sys.exit(1)

    if not options.ser_port:
        port = fu.scan_for_device()

        if not port:
            print("Unable to locate an active bootloader")
            parser.print_help()
            sys.exit(1)
    else:
        port = options.ser_port

    if not options.verbose:
        verbose_mode = False
    else:
        verbose_mode = True

    fu.update_firmware(options.input_filename, port, verbose_mode)


if __name__ == '__main__':
    main()
