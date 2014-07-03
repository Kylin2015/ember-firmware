/* 
 * File:   SettingsUT.cpp
 * Author: greener
 *
 * Created on Jun 17, 2014, 12:26:26 PM
 */

#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <fstream>

#include <Settings.h>

void VerifyDefaults(Settings& settings)
{
    if(settings.GetString(JOB_NAME_SETTING).compare("slice") != 0)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong default job name: " 
                << settings.GetString(JOB_NAME_SETTING) << std::endl;
    }
    if(settings.GetInt(LAYER_THICKNESS) != 25)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong default layer thickness: " 
                << settings.GetInt(LAYER_THICKNESS) << std::endl;
    }
    if(settings.GetDouble(MODEL_EXPOSURE) != 1.5)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong default model exposure time: " 
                << settings.GetDouble(MODEL_EXPOSURE) << std::endl;
    }
    if(settings.GetBool(IS_REGISTERED) != false)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong default isRegistered:  true " 
                 << std::endl;
    }        
}

void VerifyModSettings(Settings& settings)
{
    if(settings.GetString(JOB_NAME_SETTING).compare("WhosYerDaddy") != 0)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong new job name: " 
                << settings.GetString(JOB_NAME_SETTING) << std::endl;
    }
    if(settings.GetInt(LAYER_THICKNESS) != 42)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong new layer thickness: " 
                << settings.GetInt(LAYER_THICKNESS) << std::endl;
    }
    if(settings.GetDouble(MODEL_EXPOSURE) != 3.14)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong new model exposure time: " 
                << settings.GetDouble(MODEL_EXPOSURE)  << std::endl;
    }
    if(settings.GetBool(IS_REGISTERED) != true)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=wrong new isRegistered: false" 
                  << std::endl;
    }    
}

bool gotError = false;

class ErrorHandler: public IErrorHandler
{
    void HandleError(const char* baseMsg, bool fatal, const char* str, int value)
    {
        gotError = true;
    }  
};

void VerifyExpectedError(const char* msg)
{
    if(!gotError)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=No error from " << msg
                  << std::endl;
    }
    else
    {
       std::cout << "Got expected error from " << msg << std::endl; 
    }
    gotError = false;
}

std::string tempDir;

void Setup()
{
    tempDir = CreateTempDir();
}

void TearDown()
{
    RemoveDir(tempDir);
}

void test1() {
    std::cout << "SettingsUT test 1" << std::endl;
    
    std::string tempPath = tempDir + "/MySettings";
    std::string testSettingsPath = tempDir + "/SettingsUT";
    
    Settings settings(testSettingsPath);
    
    // verify default values
    VerifyDefaults(settings);
    
    // set some setting values
    settings.Set(JOB_NAME_SETTING, "WhosYerDaddy");
    settings.Set(LAYER_THICKNESS, "42");
    settings.Set(MODEL_EXPOSURE, "3.14");
    settings.Set(IS_REGISTERED, "true");
    
    VerifyModSettings(settings);  
    
    // verify settings not yet persisted
    std::ifstream file(testSettingsPath.c_str());

    if ( file )
    {
        std::stringstream buffer;

        buffer << file.rdbuf();

        file.close();

        if(buffer.str().find("WhosYerDaddy") != std::string::npos)
        {
            std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=Settings persisted when they shouldn't be"
                  << std::endl;
        }
    }
    
    
    // save it to a  different file
    settings.Save(tempPath);
    
    // restore and verify default values
    settings.RestoreAll();
    VerifyDefaults(settings);
    
    // load new settings from the file and verify expected values
    settings.Load(tempPath);   
    VerifyModSettings(settings); 
    
    // check restore of individual settings
    settings.Restore(JOB_NAME_SETTING);
    if(settings.GetString(JOB_NAME_SETTING).compare("slice") != 0)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=couldn't restore default job name: " 
                << settings.GetString(JOB_NAME_SETTING) << std::endl;
    }
    settings.Restore(MODEL_EXPOSURE);
    if(settings.GetDouble(MODEL_EXPOSURE) != 1.5)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=couldn't restore default model exposure time: " 
                << settings.GetDouble(MODEL_EXPOSURE)  << std::endl;
    }
    // make sure other settings not restored
    if(settings.GetInt(LAYER_THICKNESS) != 42)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=unintentionally restored layer thickness: " 
                << settings.GetInt(LAYER_THICKNESS) << std::endl;
    }
    if(settings.GetBool(IS_REGISTERED) != true)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=unintentionally restored isRegistered: false" 
                  << std::endl;
    }    
    
    // verify JSON string IO, by getting the string that has layer thickness 42
    std::string json = settings.GetAllSettingsAsJSONString();
    // restore defaults
    settings.RestoreAll();
    VerifyDefaults(settings);
    // now load from string with thickness = 42
    bool retVal = settings.LoadFromJSONString(json);
    if(!retVal)
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=Couldn't LoadFromJSONString" << std::endl;
    // and verify that it was changed
    if(settings.GetInt(LAYER_THICKNESS) != 42)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=JSON IO didn't restore layer thickness: " 
                << settings.GetInt(LAYER_THICKNESS) << std::endl;
    }

    // verify we don't change when JSON string refers to an unknown setting
    settings.RestoreAll();
    VerifyDefaults(settings);
    int pos = json.find(LAYER_THICKNESS);
    json.replace(pos, 5, "Later");
    retVal = settings.LoadFromJSONString(json);
    if(!retVal)
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=Couldn't LoadFromJSONString" << std::endl;    
    if(settings.GetInt(LAYER_THICKNESS) != 25)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=improperly changed layer thickness: " 
                << settings.GetInt(LAYER_THICKNESS) << std::endl;
    }
    
    // verify changes to the settings file only take effect after a refresh
    settings.RestoreAll();  // reset TEST_SETTINGS_PATH to its defaults
    Settings mod(tempPath);
    Copy(testSettingsPath, tempPath);
    if(mod.GetInt(LAYER_THICKNESS) != 42)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=copying a file changed a setting" 
                 << std::endl;
    }
    mod.Refresh();
    if(mod.GetInt(LAYER_THICKNESS) != 25)
    {
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=Refresh failed to load changed setting" 
                 << std::endl;
    }
    
    // test error conditions
    ErrorHandler eh;
    settings.SetErrorHandler(&eh);
    // try loading a file that doesn't exist
    settings.RestoreAll();
    gotError = false;
    settings.Load("NonExistentFile");
    VerifyExpectedError("non-existent file");
    VerifyDefaults(settings);
    
    // try loading an improperly formatted file
    settings.Load("/smith/smith");
    VerifyExpectedError("improperly formatted file");
    VerifyDefaults(settings);
    
    // try reading from a non-JSON string
    retVal = settings.LoadFromJSONString("This is clearly not a JSON settings string!");
    if(retVal)
        std::cout << "%TEST_FAILED% time=0 testname=test1 (SettingsUT) message=LoadFromJSONString returned true when it should have failed" << std::endl;
    VerifyExpectedError("improperly formatted  string");
    VerifyDefaults(settings);
    
    // attempt to save to an illegal file name
    settings.Save("badFilename*//?");
    VerifyExpectedError("saving to illegal filename");
    VerifyDefaults(settings);
    
    // attempt to save to file that's write protected
    // (for some reason, this doesn't prevent Save() from working)
//    system ("chmod 0000 " TEST_SETTINGS_PATH);
//    settings.Save();
//    VerifyExpectedError("saving to read-only file");
//    VerifyDefaults(settings);
    
    // attempt to restore a non-existent setting
    settings.Restore("ThisIsATest");
    VerifyExpectedError("restoring a non-existent setting");
    VerifyDefaults(settings);
    
    // attempt to set a non-existent setting
    settings.Set(std::string("Material"), std::string("PureUnobtanium"));
    VerifyExpectedError("setting a non-existent setting");
    VerifyDefaults(settings);
    
    // attempt to get a non-existent setting
    int x = settings.GetInt("XYZ");
    VerifyExpectedError("getting a non-existent setting");
    VerifyDefaults(settings);
    
    // attempt to get the wrong type of setting
    x = settings.GetInt(JOB_NAME_SETTING);
    VerifyExpectedError("getting string as int setting");
    VerifyDefaults(settings);
    
    // attempt to get the wrong type of setting
    double y = settings.GetDouble(JOB_NAME_SETTING);
    VerifyExpectedError("getting string as double setting");
    VerifyDefaults(settings);
      
  // no error here because any type of setting can be interpreted as a string!
     // attempt to get the wrong type of setting
//    std::string str = settings.GetString(LAYER_THICKNESS);
//    VerifyExpectedError("No error getting int as string setting");
//    VerifyDefaults(settings);

     // attempt to get a non-existent string setting
    std::string str = settings.GetString("WhosYerMama");
    VerifyExpectedError("getting non-existent string setting");
    VerifyDefaults(settings);
    
    // attempt to get a non-existent int setting
    x = settings.GetInt("WhosYerMama");
    VerifyExpectedError("getting non-existent int setting");
    VerifyDefaults(settings);
    
    // attempt to get a non-existent double setting
    y = settings.GetDouble("WhosYerMama");
    VerifyExpectedError("getting non-existent double setting");
    VerifyDefaults(settings);
    
    // attempt to get a non-existent bool setting
    bool b = settings.GetBool("WhosYerMama");
    VerifyExpectedError("getting non-existent bool setting");
    VerifyDefaults(settings);
}

int main(int argc, char** argv) {
    std::cout << "%SUITE_STARTING% SettingsUT" << std::endl;
    std::cout << "%SUITE_STARTED%" << std::endl;

    std::cout << "%TEST_STARTED% test1 (SettingsUT)" << std::endl;
    Setup();
    test1();
    TearDown();
    std::cout << "%TEST_FINISHED% time=0 test1 (SettingsUT)" << std::endl;

    std::cout << "%SUITE_FINISHED% time=0" << std::endl;

    return (EXIT_SUCCESS);
}
