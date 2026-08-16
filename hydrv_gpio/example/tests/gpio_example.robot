*** Settings ***
Suite Setup     Setup
Suite Teardown  Teardown
Test Teardown   Test Teardown
Resource        ${RENODEKEYWORDS}
Library         gpio_example_keywords.py

*** Test Cases ***
GPIO example toggles LED pin
    GPIO Example Should Blink Led
