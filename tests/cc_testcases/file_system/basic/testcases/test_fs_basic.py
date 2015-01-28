import cc_testcase

import random
import os
import string

MAX_TEST_FILE_SIZE = 23453
FileData = ""
FileList =  [
    ("test_fs_basic_0.txt", 0),
    ("test_fs_basic_1.txt", 1),
    ("test_fs_basic_2.txt", 500),
    ("test_fs_basic_3.txt", 1600),
    ("test_fs_basic_4.txt", random.randint(1601, MAX_TEST_FILE_SIZE) ),
    ("test_fs_basic_5.bin", MAX_TEST_FILE_SIZE),
    ]


# Generate a random file content
for i in xrange(MAX_TEST_FILE_SIZE):
    FileData += chr(random.randint(0, 255))

class DiscoveryTestCase(cc_testcase.TestCase):

    def test_1_upload_file_execution_path(self):
        """ Tests put and get on different sized files. This creates a series of files with random content in the execution path. """
        global FileData
        global FileList


        # Send different file content
        for fileName, fileSize in FileList:
            # Get the data chunk for each size
            eachFileUploadData = FileData[:fileSize]

            filePath = "%s" % fileName

            # Upload, Download and Verify the file
            self.auxiliar_upload_download_verify(filePath, fileSize, eachFileUploadData)



    def test_2_upload_file_root_path(self):
        """ Tests put and get a file inside the root path. """
        global FileData

        rootPath = self.tempPath

        self.log.info("self.tempPath: '%s' " % (self.tempPath) )

        # Generate the destination path for the file
        fileName = "test_fs_basic_0.txt"
        filePath = os.path.join(rootPath,fileName)
        fileSize = len(FileData)

        # Upload, Download and Verify the file
        self.auxiliar_upload_download_verify(filePath, fileSize, FileData)




    def test_3_upload_file_subdirectories_root_path(self):
        """ Tests put and get a file inside different subdirectory levels in the root path. """
        global FileData

        rootPath = self.tempPath

        for i in range(1,64):
            # Add a directory level in each loop
            rootPath = os.path.join( rootPath , "%s" % i)
            # Create the new directory on Device
            os.makedirs(rootPath)

            # Generate the destination path for the file
            fileName = "test_fs_basic_%s.txt" % i
            filePath = os.path.join(rootPath,fileName)
            fileSize = len(FileData)


            # Upload, Download and Verify each file
            self.auxiliar_upload_download_verify(filePath, fileSize, FileData)



    def test_4_upload_file_max_path_length(self):
        """ Tests put and get a file in a long path. """
        global FileData

        rootPath = self.tempPath

        for i in range(0,9):
            folderNameLevel = ''.join( random.choice(string.ascii_letters + string.digits) for i in range(255))
            rootPath = os.path.join( rootPath , folderNameLevel)


        # Create the new directory on Device
        os.makedirs(rootPath)

        # Generate the destination path for the file
        fileName = "test_fs_basic_max.txt"
        filePath = os.path.join(rootPath,fileName)
        fileSize = len(FileData)

        self.log.info("Uploading file '%s' in a path with a total length of %d" % ( fileName, len(filePath) ) )

        # Upload, Download and Verify each file
        self.auxiliar_upload_download_verify(filePath, fileSize, FileData)



    def test_5_upload_file_folder_without_rights(self):
        """ Tests put a file inside a subdirectory in the root path without write permissions. """
        global FileData

        # Generate the new path
        rootPath = self.tempPath
        rootPath = os.path.join( rootPath , "folderWithoutRights")


        # Create the new directory on Device without write permissions
        os.makedirs(rootPath, 0400)

        # Generate the destination path for the file
        fileName = "test_fs_basic_without_rights.txt"
        filePath = os.path.join(rootPath,fileName)
        fileSize = len(FileData)

        self.log.info("Uploading file '%s' in a path without rights" % ( filePath ) )

        # Send file to Device, expected result is error due to Permision denied
        result, requestResponse = self.cloudHandler.uploadFileToDevice(self.device_id, filePath, FileData)
        if (result):
            self.fail("Could upload file '%s' on device in a folder without write permissions" % (filePath) )

        else:
            # Get error string
            errorMessage = requestResponse.resource["sci_reply"]["file_system"]["device"]["commands"]["put_file"]["error"]['#text']

            if( errorMessage == "Permission denied" ):
                self.log.info("Device rejects the file '%s' due to '%s'....OK" % (fileName, errorMessage) )
            else:
                self.fail("Unknown error on device in a folder without write permissions: %s" % (requestResponse.content) )



    def test_6_upload_file_unknown_folder(self):
        """ Tests put a file in a nonexistent subdirectory. """
        global FileData

        # Generate the new path
        rootPath = self.tempPath
        rootPath = os.path.join( rootPath , "unknownFolder")

        # Generate the destination path for the file
        fileName = "test_fs_basic_unknown.txt"
        filePath = os.path.join(rootPath,fileName)
        fileSize = len(FileData)

        self.log.info("Uploading file '%s' in a unknown path" % ( filePath ) )

        # Send file to Device, expected result is error due to 'No such file or directory'
        result, requestResponse = self.cloudHandler.uploadFileToDevice(self.device_id, filePath, FileData)
        if (result):
            self.fail("Could upload file '%s' on device in an unknown folder" % (filePath) )

        else:
            # Get error string
            errorMessage = requestResponse.resource["sci_reply"]["file_system"]["device"]["commands"]["put_file"]["error"]['#text']

            if( errorMessage == "No such file or directory" ):
                self.log.info("Device rejects the file '%s' due to '%s'....OK" % (fileName, errorMessage) )
            else:
                self.fail("Unknown error on device in an unknown folder: %s" % (requestResponse.content) )



    def test_7_download_file_unknown_folder(self):
        """ Tests get a file from a nonexistent folder. """

        # Generate the new path
        rootPath = self.tempPath
        rootPath = os.path.join( rootPath , "unknownFolder")

        # Generate the destination path for the file
        fileName = "test_fs_basic_unknown.txt"
        filePath = os.path.join(rootPath,fileName)

        self.log.info("Downloading file '%s' from a unknown path" % ( filePath ) )

        # Recover file to Device, expected result is error due to Permision denied
        result, eachFileDownloadData, requestResponse = self.cloudHandler.downloadFileFromDevice(self.device_id, filePath)
        if (result):
            self.fail("Could download file '%s' on device from an unknown folder" % (filePath) )

        else:
            # Get error string
            errorMessage = requestResponse.resource["sci_reply"]["file_system"]["device"]["commands"]["get_file"]["error"]['#text']

            if( errorMessage == "No such file or directory" ):
                self.log.info("Device rejects the file '%s' due to '%s'....OK" % (fileName, errorMessage) )
            else:
                self.fail("Unknown error on device in an unknown folder: %s" % (requestResponse.content) )



    def test_8_download_file_unknown(self):
        """ Tests get a nonexistent file. """

        # Generate the destination path for the file
        fileName = "test_fs_basic_unknown.txt"
        filePath = fileName

        self.log.info("Downloading file '%s' from a unknown path" % ( filePath ) )

        # Recover file to Device, expected result is error due to Permision denied
        result, eachFileDownloadData, requestResponse = self.cloudHandler.downloadFileFromDevice(self.device_id, filePath)
        if (result):
            self.fail("Could download file '%s' on device from an unknown folder" % (filePath) )

        else:
            # Get error string
            errorMessage = requestResponse.resource["sci_reply"]["file_system"]["device"]["commands"]["get_file"]["error"]['#text']

            if( errorMessage == "No such file or directory" ):
                self.log.info("Device rejects the file '%s' due to '%s'....OK" % (fileName, errorMessage) )
            else:
                self.fail("Unknown error on device in an unknown folder: %s" % (requestResponse.content) )




    def test_9_download_file_without_read_rights(self):
        """ Tests get a file without read permissions inside a subdirectory in the root path. """
        global FileData

        # Generate the new path
        rootPath = self.tempPath
        rootPath = os.path.join( rootPath , "folderWithRights")


        # Create the new directory on Device
        os.makedirs(rootPath)

        # Generate the destination path for the file
        fileName = "test_fs_basic_without_rights.txt"
        filePath = os.path.join(rootPath,fileName)
        fileSize = len(FileData)

        # Generate the file on Device
        handlerFile = open(filePath, "w")
        handlerFile.write(FileData)
        handlerFile.close()

        # Remove read rights from file
        os.chmod(filePath, 0)

        self.log.info("Downloading file '%s' from a path without read rights" % ( filePath ) )

        # Recover file to Device, expected result is error due to Permision denied
        result, eachFileDownloadData, requestResponse = self.cloudHandler.downloadFileFromDevice(self.device_id, filePath)
        if (result):
            self.fail("Could download file '%s' from device in a folder without read permissions" % (filePath) )

        else:
            # Get error string
            errorMessage = requestResponse.resource["sci_reply"]["file_system"]["device"]["commands"]["get_file"]["error"]['#text']

            if( errorMessage == "Permission denied" ):
                self.log.info("Device rejects read the file '%s' due to '%s'....OK" % (fileName, errorMessage) )
            else:
                self.fail("Unknown error on device in a folder without write permissions: %s" % (requestResponse.content) )



    def auxiliar_upload_download_verify(self, filePath, fileSize, fileData):

        # Step 1: Send file
        self.log.info("Uploading file '%s' with size %s to device..." % (filePath, fileSize) )
        result, requestResponse = self.cloudHandler.uploadFileToDevice(self.device_id, filePath, fileData)
        if (result):
            self.log.info("File '%s' with size %s was successfully uploaded on device" % (filePath, fileSize) )
        else:
            self.fail("Could not upload file '%s' with size %s on device: %s" % (filePath, fileSize, requestResponse.content) )


        # Step 2: Get file
        self.log.info("Downloading file '%s' with size %s from device..." % (filePath, fileSize) )
        result, eachFileDownloadData, requestResponse = self.cloudHandler.downloadFileFromDevice(self.device_id, filePath)
        if (result):
            self.log.info("File '%s' with size %s was successfully downloaded from device" % (filePath, fileSize) )
        else:
            self.fail("Could not download file '%s' with size %s on device: %s" % (filePath, fileSize, requestResponse.content) )


        # Step 3: Verify if file content match with the original
        if ( fileData == eachFileDownloadData ):
            self.log.info("Content of file '%s' match!" % (filePath) )
        else:
            self.fail("Content of file '%s' mismatch!!!\n Original Content: '%s'\n Downloaded Content: '%s'" % (filePath, fileData, eachFileDownloadData) )



if __name__ == '__main__':
    unittest.main()
