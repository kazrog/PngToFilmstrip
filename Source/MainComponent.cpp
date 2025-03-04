//Copyright 2023 Julien CAUBET
//
//Licensed under the Apache License, Version 2.0 (the "License");
//you may not use this file except in compliance with the License.
//You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//Unless required by applicable law or agreed to in writing, software
//distributed under the License is distributed on an "AS IS" BASIS,
//WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//See the License for the specific language governing permissions and
//limitations under the License.

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize (600, 400);
    setName("Png To Filmstrip");

    addAndMakeVisible(logoOpenWebBrowser);
    addAndMakeVisible(appNameLanel);

    addAndMakeVisible(inputFilesLabel);
    addAndMakeVisible(inputTextEditor);
    addAndMakeVisible(inputBrowserButton);

    addAndMakeVisible(outputFolderLabel);
    addAndMakeVisible(outputTextEditor);
    addAndMakeVisible(outputBrowserButton);

    addAndMakeVisible(supportUsButton);
    addAndMakeVisible(proceedButton);

    appNameLanel.setText("Png To Filmstrip", juce::dontSendNotification);
    inputFilesLabel.setText("Select the folder containing the .png files:", juce::dontSendNotification);
    inputTextEditor.setText(fileToLoad.getFullPathName(), juce::dontSendNotification);
    outputFolderLabel.setText("Select the output folder:", juce::dontSendNotification);
    outputTextEditor.setText(fileToSave.getFullPathName(), juce::dontSendNotification);
    supportUsButton.setButtonText("Support us!");
    proceedButton.setButtonText("Let's convert to filmstrip!");

    inputBrowserButton.onClick = [&] {launchInputBrowser(); };
    outputBrowserButton.onClick = [&] {launchOutputBrowser(); };
    supportUsButton.onClick = []{juce::URL("https://juqasolutions.github.io/supportus.html").launchInDefaultBrowser();};
    proceedButton.onClick = [this] {createFilmstripThread.launchThread();};
    logoOpenWebBrowser.onClick = []{juce::URL("https://github.com/JuqaSolutions/PngToFilmstrip").launchInDefaultBrowser();};

    appNameLanel.setJustificationType(juce::Justification::centred);
    appNameLanel.setColour(juce::Label::textColourId, appNameColour);
    appNameLanel.setFont(juce::Font(24.0f, juce::Font::bold | juce::Font::italic));

    inputFilesLabel.setJustificationType(juce::Justification::centred);
    inputFilesLabel.setColour(juce::Label::textColourId, browserLabelColour);
    inputFilesLabel.setFont(juce::Font(14.0f, juce::Font::bold));

    outputFolderLabel.setJustificationType(juce::Justification::centred);
    outputFolderLabel.setColour(juce::Label::textColourId, browserLabelColour);
    outputFolderLabel.setFont(juce::Font(14.0f, juce::Font::bold));

    supportUsButton.setColour(juce::TextButton::buttonColourId, backgroundColour);

    initializeImageButtons();
}

MainComponent::~MainComponent()
{
    if(createFilmstripThread.isThreadRunning())
        createFilmstripThread.stopThread(10);
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (backgroundColour);

    auto bounds = getLocalBounds();
    auto height = bounds.getHeight() / 4;
    bounds.removeFromTop(height);
    auto inputBounds = bounds.removeFromTop(height);
    auto outputBounds = bounds.removeFromTop(height);

    g.setColour(browserBackgroundColour);
    g.fillRoundedRectangle(inputBounds.removeFromBottom(height / 2).toFloat(), 20);
    g.fillRoundedRectangle(outputBounds.removeFromBottom(height / 2).toFloat(), 20);

}

void MainComponent::resized()
{
    auto bounds = getLocalBounds();
    auto height = bounds.getHeight() / 4;

    auto headerBounds = bounds.removeFromTop(height);
    auto inputBounds = bounds.removeFromTop(height);
    auto outputBounds = bounds.removeFromTop(height);
    auto footerBounds = bounds.removeFromTop(height);

    appNameLanel.setBounds(headerBounds.removeFromRight(headerBounds.getWidth() / 2));
    logoOpenWebBrowser.setBounds(headerBounds.reduced(height * 0.2f));

    inputFilesLabel.setBounds(inputBounds.removeFromTop(inputBounds.getHeight() / 2));
    inputTextEditor.setBounds(inputBounds.removeFromLeft(inputBounds.getWidth() * 0.9f));
    inputBrowserButton.setBounds(inputBounds.reduced(height * 0.07f));

    outputFolderLabel.setBounds(outputBounds.removeFromTop(outputBounds.getHeight() / 2));
    outputTextEditor.setBounds(outputBounds.removeFromLeft(outputBounds.getWidth() * 0.9f));
    outputBrowserButton.setBounds(outputBounds.reduced(height * 0.07f));

    supportUsButton.setBounds(footerBounds.removeFromLeft(footerBounds.getWidth() / 2).reduced(height * 0.3f));
    proceedButton.setBounds(footerBounds.reduced(height * 0.3f));
}

void MainComponent::launchInputBrowser()
{
    myChooser = std::make_unique<juce::FileChooser>("Select the folder containing the .png files:", fileToLoad);

    myChooser->launchAsync(
            juce::FileBrowserComponent::openMode |
                    juce::FileBrowserComponent::canSelectDirectories |
                    juce::FileBrowserComponent::doNotClearFileNameOnRootChange,
            [&](const juce::FileChooser& chooser)
            {
                juce::File returnedFile(chooser.getResult());
                if (returnedFile.getFileName().length() != 0)
                {
                    inputTextEditor.setText(returnedFile.getFullPathName(), juce::dontSendNotification);
                    fileToLoad = returnedFile;
                }

                if (fileToLoad.findChildFiles(
                        2,
                        false,
                        "*.png").size() < 2)
                {
                    juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                                                           "Wrong folder?",
                                                           "It seems that your selected folder contains less than 2 .png file. It needs at least 2 file to do the job.");
                }
            });
}

void MainComponent::launchOutputBrowser()
{
    myChooser = std::make_unique<juce::FileChooser>("Select the output folder:", fileToSave, "*.png");

    myChooser->launchAsync(
            juce::FileBrowserComponent::saveMode |
            juce::FileBrowserComponent::canSelectFiles |
            juce::FileBrowserComponent::warnAboutOverwriting |
            juce::FileBrowserComponent::useTreeView |
            juce::FileBrowserComponent::doNotClearFileNameOnRootChange,
            [&](const juce::FileChooser& chooser)
            {
                juce::File returnedFile(chooser.getResult());
                if (returnedFile.getFileName().length() != 0)
                {
                    outputTextEditor.setText(returnedFile.getFullPathName(), juce::dontSendNotification);
                    fileToSave = returnedFile;
                }
            });
}

void MainComponent::proceed()
{
    DBG("Proceed() is loaded: " << fileToLoad.getFullPathName());

    if (!fileToLoad.exists()) {
        fileToLoad.createDirectory();
    }

    if (!fileToSave.exists()) {
        fileToSave.create();
    }
    else
    {
        fileToSave.deleteFile();
        fileToSave.create();
    }

    if (fileToLoad.isDirectory()) {
        DBG("is directory");
        juce::Array<juce::File> result;
        
        // Get all PNG files in the directory
        fileToLoad.findChildFiles(result, juce::File::findFiles, false, "*.png");
        
        // Custom compare function for numeric filenames
        auto compareNumericFilenames = [](const juce::File& file1, const juce::File& file2) -> int {
            // Extract the base filenames without extension
            juce::String name1 = file1.getFileNameWithoutExtension();
            juce::String name2 = file2.getFileNameWithoutExtension();
            
            // Find where the numbers start in each filename
            int idx1 = name1.length() - 1;
            while (idx1 >= 0 && juce::CharacterFunctions::isDigit(name1[idx1]))
                --idx1;
            
            int idx2 = name2.length() - 1;
            while (idx2 >= 0 && juce::CharacterFunctions::isDigit(name2[idx2]))
                --idx2;
            
            // If both filenames have numbers at the end and share the same prefix
            if (idx1 >= 0 && idx2 >= 0) {
                juce::String prefix1 = name1.substring(0, idx1 + 1);
                juce::String prefix2 = name2.substring(0, idx2 + 1);
                
                if (prefix1 == prefix2) {
                    int num1 = name1.substring(idx1 + 1).getIntValue();
                    int num2 = name2.substring(idx2 + 1).getIntValue();
                    return num1 - num2;
                }
            }
            
            // Default to lexicographical comparison
            return name1.compare(name2);
        };
        
        // Manual bubble sort since JUCE Array might not support custom comparators in older versions
        for (int i = 0; i < result.size() - 1; ++i) {
            for (int j = 0; j < result.size() - i - 1; ++j) {
                if (compareNumericFilenames(result[j], result[j + 1]) > 0) {
                    result.swap(j, j + 1);
                }
            }
        }
        
        // Log the sorted files for debugging
        DBG("Sorted files order:");
        for (int i = 0; i < result.size(); ++i) {
            DBG(result[i].getFileName());
        }
        
        if (result.size() > 0) {
            DBG("contains of the directory: " << result.size());
            
            juce::PNGImageFormat imageFileFormat;
            juce::Array<juce::Image> imageArray{};
            int width = -1;
            int height = -1;
            int outputHeight = 0;

            for (juce::File file: result)
            {
                juce::Image tempImage;
                int tempWidth{0};
                int tempHeight{0};

                tempImage = juce::ImageFileFormat::loadFrom(file);

                if (!tempImage.isValid())
                {
                    juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                                                           "File not valid",
                                                           "It seems that at least one file isn't valid.");
                    return;
                }
                tempWidth = tempImage.getWidth();
                tempHeight = tempImage.getHeight();

                if (width == -1)
                {
                    width = tempWidth;
                    height = tempHeight;
                }
                else
                {
                    if (tempWidth != width || tempHeight != height)
                    {
                        juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                                                               "Wrong dimensions",
                                                               "All the .png files must have the same dimensions");
                        return;
                    }
                }

                imageArray.add(tempImage);
            }

            outputHeight = height * imageArray.size();

            juce::Image outputImage{juce::Image::ARGB, width, outputHeight, true};

            int arrayIndex = 0;
            int heightForLoop = 0;

            while (arrayIndex < imageArray.size()) {
                for (int h = 0; h < height; ++h) {
                    for (int w = 0; w < width; ++w) {
                        outputImage.setPixelAt(w, heightForLoop, imageArray.getReference(arrayIndex).getPixelAt(w, h));
                    }
                    ++heightForLoop;
                }
                ++arrayIndex;
            }

            juce::FileOutputStream outputStream{fileToSave};
            imageFileFormat.writeImageToStream(outputImage, outputStream);
        }
        else
        {
            juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,
                                                   "Empty folder",
                                                   "The folder containing the .png files is empty. Or your files aren't .png type");
        }
    }
}

void MainComponent::initializeImageButtons()
{
    juce::Image logo = juce::ImageCache::getFromMemory(BinaryData::juqa_logo_png, BinaryData::juqa_logo_pngSize);
    logoOpenWebBrowser.setImages(false,
                                 true,
                                 true,
                                 logo,
                                 1.0f,
                                 juce::Colours::transparentBlack,
                                 logo,
                                 1.0f,
                                 juce::Colours::transparentBlack,
                                 logo,
                                 1.0f,
                                 juce::Colours::transparentBlack
                                 );

    juce::Image inputBrowser = juce::ImageCache::getFromMemory(BinaryData::Search_Logo_png, BinaryData::Search_Logo_pngSize);
    inputBrowserButton.setImages(false,
                                 true,
                                 true,
                                 inputBrowser,
                                 1.0f,
                                 juce::Colours::transparentBlack,
                                 inputBrowser,
                                 1.0f,
                                 juce::Colours::transparentBlack,
                                 inputBrowser,
                                 1.0f,
                                 juce::Colours::transparentBlack
    );

    juce::Image outputBrowser = juce::ImageCache::getFromMemory(BinaryData::Search_Logo_png, BinaryData::Search_Logo_pngSize);
    outputBrowserButton.setImages(false,
                                 true,
                                 true,
                                  outputBrowser,
                                 1.0f,
                                 juce::Colours::transparentBlack,
                                  outputBrowser,
                                 1.0f,
                                 juce::Colours::transparentBlack,
                                  outputBrowser,
                                 1.0f,
                                 juce::Colours::transparentBlack
    );
}

const juce::Label &MainComponent::getAppNameLanel() const {
    return appNameLanel;
}

const juce::Label &MainComponent::getInputFilesLabel() const {
    return inputFilesLabel;
}

const juce::Label &MainComponent::getOutputFolderLabel() const {
    return outputFolderLabel;
}

const juce::TextButton &MainComponent::getProceedButton() const {
    return proceedButton;
}

const juce::TextButton &MainComponent::getSupportUsButton() const {
    return supportUsButton;
}

const juce::ImageButton &MainComponent::getLogoOpenWebBrowser() const {
    return logoOpenWebBrowser;
}

const juce::ImageButton &MainComponent::getInputBrowserButton() const {
    return inputBrowserButton;
}

const juce::ImageButton &MainComponent::getOutputBrowserButton() const {
    return outputBrowserButton;
}

const juce::Label &MainComponent::getInputTextEditor() const {
    return inputTextEditor;
}

const juce::Label &MainComponent::getOutputTextEditor() const {
    return outputTextEditor;
}

const juce::File &MainComponent::getFileToLoad() const {
    return fileToLoad;
}

const juce::File &MainComponent::getFileToSave() const {
    return fileToSave;
}

const std::unique_ptr<juce::FileChooser> &MainComponent::getMyChooser() const {
    return myChooser;
}
