
#pragma once
#include <JuceHeader.h>
namespace Akasha {
	inline void printFileTree(const juce::File& directory, int indent = 0) {
		// Print the current directory/file
		juce::String indentation(" ", indent * 2); // Indentation for hierarchy
		DBG(indentation + directory.getFileName()); // Print file/directory name

		if (directory.isDirectory()) {
			// Recursively print child files/directories
			auto childFiles = directory.findChildFiles(juce::File::findFilesAndDirectories, false);
			for (auto& child : childFiles) {
				printFileTree(child, indent + 1);
			}
		}
	}

	inline void packDirectory(const juce::File& sourceDir, const juce::File& outputFile) {
		juce::FileOutputStream outputStream(outputFile);
		if (!outputStream.openedOk()) {
			DBG("Failed to open output file for packing: " + outputFile.getFullPathName());
			return;
		}

		// Recursively get all files in the source directory
		juce::DirectoryIterator iter(sourceDir, true, "*", juce::File::findFiles);
		while (iter.next()) {
			auto file = iter.getFile();
			auto relativePath = file.getRelativePathFrom(sourceDir);

			// Write file path length and path
			outputStream.writeInt((int)relativePath.length());
			outputStream.writeText(relativePath, false, false, nullptr);

			// Write file size and content
			juce::MemoryBlock fileData;
			file.loadFileAsData(fileData);
			outputStream.writeInt((int)fileData.getSize());
			outputStream.write(fileData.getData(), fileData.getSize());
		}

		outputStream.flush();
		DBG("Packed directory into: " + outputFile.getFullPathName());
	}

	// Function to unpack files from the binary file
	inline void unpackToDirectory(const juce::File& packedFile, const juce::File& outputDir) {
		juce::FileInputStream inputStream(packedFile);
		if (!inputStream.openedOk()) {
			DBG("Failed to open packed file: " + packedFile.getFullPathName());
			return;
		}

		while (!inputStream.isExhausted()) {
			// Read file path length
			int pathLength = inputStream.readInt();

			// Read the file path as a UTF-8 encoded string
			std::vector<char> pathBuffer(pathLength);
			inputStream.read(pathBuffer.data(), pathLength);
			juce::String relativePath = juce::String::fromUTF8(pathBuffer.data(), pathLength);

			// Read file size
			int fileSize = inputStream.readInt();

			// Read file content
			juce::MemoryBlock fileData;
			inputStream.readIntoMemoryBlock(fileData, fileSize);

			// Write the file to the output directory
			auto outputFile = outputDir.getChildFile(relativePath);
			outputFile.getParentDirectory().createDirectory();
			outputFile.replaceWithData(fileData.getData(), fileData.getSize());
		}

		DBG("Unpacked files to: " + outputDir.getFullPathName());
	}


	inline void unpackBinaryDataToTemp(const void* packedData, size_t packedSize, const juce::File& outputDir) {
		// Create the output directory if it doesn't exist
		if (!outputDir.exists())
			outputDir.createDirectory();

		// Create an input stream for the binary data
		juce::MemoryInputStream inputStream(packedData, packedSize, false);

		while (!inputStream.isExhausted()) {
			// Read file path length
			int pathLength = inputStream.readInt();

			// Read the file path
			std::vector<char> pathBuffer(pathLength);
			inputStream.read(pathBuffer.data(), pathLength);
			juce::String relativePath = juce::String::fromUTF8(pathBuffer.data(), pathLength);

			// Read file size
			int fileSize = inputStream.readInt();

			// Read file content
			juce::MemoryBlock fileData;
			inputStream.readIntoMemoryBlock(fileData, fileSize);

			// Write the file to the output directory
			auto outputFile = outputDir.getChildFile(relativePath);
			outputFile.getParentDirectory().createDirectory(); // Ensure parent directories exist
			outputFile.replaceWithData(fileData.getData(), fileData.getSize());
		}

		DBG("Unpacked files to: " + outputDir.getFullPathName());
	}
}