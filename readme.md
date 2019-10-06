# Sphere B2C
[![Build Status](https://dev.azure.com/fred27murphy/SphereB2C/_apis/build/status/FredMurphy.SphereB2C?branchName=master)](https://dev.azure.com/fred27murphy/SphereB2C/_build/latest?definitionId=2&branchName=master)
This is a multi-project solution to demonstrate the use of a Microsoft Azure Sphere device to provide custom Multi-Factor Authentication in Azure AD B2C.

Please note that this is written as demonstration code. It is designed to illustrate the concepts as simply as possible. For this reason, it is not production quality code.

## Project Components

### SphereB2CWeb
This is a simple ASP.NET Core website based on the template that is provided in Visual Studio 2019. The changes I have made are to:
 - secure the application with OpenID Connect (using Azure AD B2C)
 - create a page to display the user's claims
 - secure a single admin page using our custom MFA claims

The site is also hosted at https://sphereb2cweb.azurewebsites.net/. On the front page you will see a list of accounts you can use to log in.

### Identity Experience Framework (Azure AD B2C)
Azure AD B2C is configured to allow the registration of users with a few extension attributes. These are:
- **secondaryMethod** This is the way we want to do our secondary authentication. The values supported in the sample project are _none_, _nfc_ (an NFC tag) and _button_ (a simple button press on the Avnet Azure Sphere Development Kit).
- **expectedValue** For nfc this should be the NFC tag ID (in hex, no spaces). For button it should be _button_a_ or _button_b_. (_button_z_ is a secret "backdoor" that will always appear to be pressed)
- **deviceName** This is the device name assigned to the Azure Sphere device so it can be found in Azure IoT Hub.

The Identity Experience Framework XML files configure storing these attributes and also how we call out to our Azure Function during login.

### SphereB2CFunction
This Azure Function expects some parameters passed in from our Azure AD B2C webhook - the device name to pass the request to, the method (nfc or button) we're using and the value to match (e.g. NFC tag ID). This will then call a direct method on the Azure Sphere device via Azure IoT Hub.

### SphereB2CDevice
Our Azure Sphere device is expected to respond to a direct method called Authenticate (with a secondaryMethod parameter of _nfc_ or _button_) and respond with the appropriate value to match what we've stored in Azure B2C.

If it receives a request for a simple button-press confirmation it will wait a mazimum of 10 seconds for a button to be pressed and confirm whether this was _button_a_ or _button_b_.

If it receives a request for a NFC tag then it uses the PN7120-based NFC click board to read the ID of any NFC tag it can find. Once again, 10s is the timeout here to prevent our web application seeming too unresponsive.

An optional Click OLED-B display will give a prompt when you're expected to provide your authentication.
