from xbvc_py import *
import serial
from ihex import ihex
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
        self._ser.close()


    def _read(self, length):
        res = self._ser.read(length)

        if len(res):
            return [ord(x) for x in res]
        else:
            return None

    def _write(self, data):
        st_data = ''.join([chr(x) for x in data])
        self._ser.write(st_data)

def progress_bar(width, percent_full):
    """
    Prints a progress bar in the form [*****        ] xx%
    """
    numstars = int(width * (percent_full / 100.))
    starstr = numstars * '*'
    starstr += (width - len(starstr)) * ' '
    progress_str = '\r[' + starstr + '] {}%'.format(int(percent_full))
    sys.stdout.write(progress_str)
    sys.stdout.flush()

def main():
    parser = optparse.OptionParser()
    parser.add_option('-f', '--filename',
                      dest="input_filename",
                      help="Filename of .ihex file to process")
      
    parser.add_option('-p', '--port',
                      dest="ser_port",
                      help="path to serial port we're talking on ex: /dev/ttyACM0")
    
    parser.add_option('-v', '--verbose',
                      dest="verbose",
                      help="turn on verbose debugging")

    options, remainder = parser.parse_args()

    if not options.input_filename:
        print "You must provide an input filename"
        parser.print_help()
        sys.exit(1)

    if not options.ser_port:
        print "Please provide a serial port"
        parser.print_help()
        sys.exit(1)

    #Create our edgepoint
    ep = XBVCSerialEP(options.ser_port)

    #set the timeout to 1 second
    # (since we're erasing, this can take some time)
    ep.timeout_ms = 1000
    ep.connect()
    ep.start()
    
    #Load and process the ihex file
    ih = ihex.iHex()
    ih.load_ihex(options.input_filename)

    #Get a list of the words in the ihex file (and their offsets)
    wl = ih.get_u32_list()

    #Now, create 64 word chunks from this list of words
    wordchunks = ihex.chunks(wl, 64)

    print "Updating firmware..."
    
    #get the total number of chunks (we'll use this for the progress bar)
    num_chunks = len([x for x in ihex.chunks(wl, 64)])
    chunks_remaining = num_chunks

    #Take this list of chunks and turn each chunk into a flash_command
    for chunk in wordchunks:
        fcmd = flash_command()

        #set the starting offset of this command to the offset of the
        #first word in the chunk
        fcmd.offset = chunk[0][0]

        #Set the data_length to the number of words in this chunk
        fcmd.data_len = len(chunk)

        #Now move the data words in this chunk to the data section of
        #the flash command
        chunkdata = [x[1] for x in chunk]
        for i in range(len(chunkdata)):
            fcmd.data[i] = chunkdata[i]

        #if we're in verbose mode, print everything we see on the
        #wire, otherwise, just keep a progress bar rolling
        if options.verbose:
            print fcmd
            print len(fcmd.encode())
        else:
            percent_complete = 100 - ((chunks_remaining / float(num_chunks)) * 100)
            progress_bar(50, percent_complete)
            
        #send the command and wait for a response
        res = ep.send(fcmd, FLASH_RESPONSE_ID)

        if res == None:
            #Retry one time
            if options.verbose:
                print "Retrying"
            ep.flush()
            res = ep.send(fcmd, FLASH_RESPONSE_ID)
            if res == None:
                raise Exception("Error flashing device, no response!")

            if options.verbose:
                print res
        elif res.error != 0:
            raise Exception("Error flasing device, got error:{}".format(res.error))
        else:
            chunks_remaining -= 1
            if options.verbose:
                print res

    if not options.verbose:
         progress_bar(50, 100)
         print "\n"
    msg = verify_command()

    #wait for a verify response
    print "Verifying firmware"
    rsp = ep.send(msg, VERIFY_RESPONSE_ID)

    if options.verbose:
        print rsp

    if rsp.error == 0:
        print "Firmware was valid, starting new firmware"
        msg = run_command()
        #TODO: Make the firmware send a response (even if it means a delay)
        ep.send(msg, RUN_RESPONSE_ID)
        ep.flush()
        ep.send(msg, RUN_RESPONSE_ID)
        ep.flush()
        ep.send(msg, RUN_RESPONSE_ID)
    else:
        raise Exception("Error validating flash on device. Updgrade failed")

    ep.stop()
    ep.disconnect()
    
if __name__ == '__main__':
    main()


    
