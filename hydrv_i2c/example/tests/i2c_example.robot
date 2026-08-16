*** Settings ***
Suite Setup     Setup
Suite Teardown  Teardown
Test Teardown   Test Teardown
Resource        ${RENODEKEYWORDS}
Library         i2c_example_keywords.py

*** Test Cases ***
I2C example reads angle over bus
    I2C Example Should Read Angle
