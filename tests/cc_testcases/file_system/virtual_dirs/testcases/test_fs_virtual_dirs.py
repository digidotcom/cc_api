import cc_testcase

import os

class DiscoveryTestCase(cc_testcase.TestCase):

    def test_1_add_virtual_directory_nonexistent_path(self):
        """ Test to create a virtual directory over a nonexistent path. """

        rootPath = self.tempPath
        rootPath = os.path.join( rootPath , "test_fs_virtual_dirs_1_folder")

        # Step 1: Send Device Request
        target = "test_fs_virtual_dirs_1"

        self.log.info("Send request to execute the source code for target '%s'..." % (target) )
        result, requestResponse = self.cloudHandler.sendDeviceRequest(self.device_id, target, rootPath)
        if (result):
            self.log.info("Response was successfully from device")
        else:
            self.fail("Error was found in the response: %s" % (requestResponse.content) )


        # Step 2: Looking for message
        expectedPattern = "test_fs_virtual_dirs_1: Error caught CCAPI_FS_ERROR_NOT_A_DIR"
        result,indexPattern,dataBuffer = self.deviceHandler.readUntilPattern ( pattern=expectedPattern, timeout=30)
        if ( not result ):
            self.fail("Console feedback was NOT arrived")
        else:
            self.log.info("Obtained expected feedback from device: '%s'" % expectedPattern)


    def test_2_add_virtual_directory_existent_path(self):
        """ Test to create a virtual directory over an existent path. """

        # Create the new directory on Device
        rootPath = self.tempPath
        rootPath = os.path.join( rootPath , "test_fs_virtual_dirs_1_folder")
        os.makedirs(rootPath)

        # Step 1: Send Device Request
        target = "test_fs_virtual_dirs_2"

        self.log.info("Send request to execute the source code for target '%s'..." % (target) )
        result, requestResponse = self.cloudHandler.sendDeviceRequest(self.device_id, target, rootPath)
        if (result):
            self.log.info("Response was successfully from device")
        else:
            self.fail("Error was found in the response: %s" % (requestResponse.content) )


        # Step 2: Looking for message
        expectedPattern = "test_fs_virtual_dirs_2: Error caught CCAPI_FS_ERROR_NONE"
        result,indexPattern,dataBuffer = self.deviceHandler.readUntilPattern ( pattern=expectedPattern, timeout=30)
        if ( not result ):
            self.fail("Console feedback was NOT arrived")
        else:
            self.log.info("Obtained expected feedback from device: '%s'" % expectedPattern)





if __name__ == '__main__':
    unittest.main()
