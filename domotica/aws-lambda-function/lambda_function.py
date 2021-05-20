# -*- coding: utf-8 -*-

# Copyright 2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
#
# Licensed under the Amazon Software License (the "License"). You may not use this file except in
# compliance with the License. A copy of the License is located at
#
#    http://aws.amazon.com/asl/
#
# or in the "license" file accompanying this file. This file is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, express or implied. See the License for the specific
# language governing permissions and limitations under the License.

import boto3
import json
from alexa.skills.smarthome import AlexaResponse
import socket
import sys
import os
import time


aws_dynamodb = boto3.client('dynamodb')

# Function to manage Alexa Request
def lambda_handler(request, context):

    # Dump the request for logging - check the CloudWatch logs
    print('lambda_handler request  -----')
    print(json.dumps(request))

    if context is not None:
        print('lambda_handler context  -----')
        print(context)

    # Validate we have an Alexa directive
    if 'directive' not in request:
        aer = AlexaResponse(
            name='ErrorResponse',
            payload={'type': 'INVALID_DIRECTIVE',
                     'message': 'Missing key: directive, Is the request a valid Alexa Directive?'})
        return send_response(aer.get())

    # Check the payload version
    payload_version = request['directive']['header']['payloadVersion']
    if payload_version != '3':
        aer = AlexaResponse(
            name='ErrorResponse',
            payload={'type': 'INTERNAL_ERROR',
                     'message': 'This skill only supports Smart Home API version 3'})
        return send_response(aer.get())

    # Crack open the request and see what is being requested
    name = request['directive']['header']['name']
    namespace = request['directive']['header']['namespace']

    print('Name:' + name)
    print('Namespace:' + namespace)
    
    # Handle the incoming request from Alexa based on the namespace

    if namespace == 'Alexa.Authorization':

        if name == 'AcceptGrant':

            # Note: This sample accepts any grant request
            # In your implementation you would use the code and token to get and store access tokens
            grant_code = request['directive']['payload']['grant']['code']
            grantee_token = request['directive']['payload']['grantee']['token']
            aar = AlexaResponse(namespace='Alexa.Authorization', name='AcceptGrant.Response')
            return send_response(aar.get())

    if namespace == 'Alexa.Discovery':

        if name == 'Discover':

            adr = AlexaResponse(namespace='Alexa.Discovery', name='Discover.Response')

            capability_alexa = adr.create_payload_endpoint_capability()

            capability_alexa_powercontroller = adr.create_payload_endpoint_capability(
                interface='Alexa.PowerController',
                supported=[{'name': 'powerState'}])

            capability_alexa_temperaturesensor = adr.create_payload_endpoint_capability(
                interface='Alexa.TemperatureSensor',
                supported=[{'name': 'temperature'}],
                retrievable='true',
                proactively_reported='true',
                )

            capability_alexa_endpointhealth = adr.create_payload_endpoint_capability(
                interface='Alexa.EndpointHealth',
                supported=[{'name': 'connectivity'}],
                proactively_reported='true',
                retrievable='true'
            )

        #    adr.add_payload_endpoint(
        #        friendly_name='Termostato Sala',
        #        endpoint_id='temperaturesensor-001',
        #        description='Termostato Sala',
        #        display_categories=["TEMPERATURE_SENSOR"],
        #        manufacturer_name = 'myself manufacter',
        #        capabilities=[capability_alexa, capability_alexa_temperaturesensor, capability_alexa_endpointhealth ])

        #    adr.add_payload_endpoint(
        #        friendly_name='Termostato Cucina',
        #        endpoint_id='temperaturesensor-002',
        #        description='Termostato Cucina',
        #        display_categories=["TEMPERATURE_SENSOR"],
        #        manufacturer_name = 'myself manufacter',
        #        capabilities=[capability_alexa, capability_alexa_temperaturesensor, capability_alexa_endpointhealth ])


            adr.add_payload_endpoint(
                friendly_name='Luce 01',
                endpoint_id='light-001',
                description='Luce 01',
                display_categories=["LIGHT"],
                manufacturer_name = 'myself manufacter',
                capabilities=[capability_alexa, capability_alexa_powercontroller])

            adr.add_payload_endpoint(
                 friendly_name='Luce 02',
                 endpoint_id='light-002',
                 description='Luce 02',
                 display_categories=["LIGHT"],
                 manufacturer_name = 'myself manufacter',
                 capabilities=[capability_alexa, capability_alexa_powercontroller])

            adr.add_payload_endpoint(
                 friendly_name='Luce 03',
                 endpoint_id='light-003',
                 description='Luce 03',
                 display_categories=["LIGHT"],
                 manufacturer_name = 'myself manufacter',
                 capabilities=[capability_alexa, capability_alexa_powercontroller])

            adr.add_payload_endpoint(
                 friendly_name='Luce 04',
                 endpoint_id='light-004',
                 description='Luce 04',
                 display_categories=["LIGHT"],
                 manufacturer_name = 'myself manufacter',
                 capabilities=[capability_alexa, capability_alexa_powercontroller])
            
            adr.add_payload_endpoint(
                 friendly_name='Luce 05',
                 endpoint_id='light-005',
                 description='Luce 05',
                 display_categories=["LIGHT"],
                 manufacturer_name = 'myself manufacter',
                 capabilities=[capability_alexa, capability_alexa_powercontroller])

            adr.add_payload_endpoint(
                 friendly_name='Luce 06',
                 endpoint_id='light-006',
                 description='Luce 06',
                 display_categories=["LIGHT"],
                 manufacturer_name = 'myself manufacter',
                 capabilities=[capability_alexa, capability_alexa_powercontroller])

            adr.add_payload_endpoint(
                 friendly_name='Luce 07',
                 endpoint_id='light-007',
                 description='Luce 07',
                 display_categories=["LIGHT"],
                 manufacturer_name = 'myself manufacter',
                 capabilities=[capability_alexa, capability_alexa_powercontroller])

            adr.add_payload_endpoint(
                 friendly_name='Luce 08',
                 endpoint_id='light-008',
                 description='Luce 08',
                 display_categories=["LIGHT"],
                 manufacturer_name = 'myself manufacter',
                 capabilities=[capability_alexa, capability_alexa_powercontroller])

            adr.add_payload_endpoint(
                 friendly_name='Cancellino',
                 endpoint_id='gate-001',
                 description='Cancellino',
                 display_categories=["DOOR"],
                 manufacturer_name = 'myself manufacter',
                 capabilities=[capability_alexa, capability_alexa_powercontroller])


            return send_response(adr.get())

    if namespace == 'Alexa':
        
        if name == 'ReportState':
            
            print('Richiesta tipo ReportState  -----')
        
            # Note: This sample always returns a success response for either a request to TurnOff or TurnOn
            endpoint_id = request['directive']['endpoint']['endpointId']
        
            print('ENDPOINT ID: ' + endpoint_id)
            
            correlation_token = request['directive']['header']['correlationToken']
            token = request['directive']['endpoint']['scope']['token']
            
            print('Correlation_Token: ' + correlation_token)
            print('Token: ' + token)
            
            apcr = AlexaResponse(name='StateReport', endpoint_id=endpoint_id, correlation_token=correlation_token, token=token)
            
            if(endpoint_id == 'temperaturesensor-001'):
            
                apcr.add_context_property(namespace='Alexa.TemperatureSensor', name='temperature', value={'value': '19.9', 'scale' : 'CELSIUS'})
                
            elif(endpoint_id == 'temperaturesensor-002'):
            
                apcr.add_context_property(namespace='Alexa.TemperatureSensor', name='temperature', value={'value': '22.3', 'scale' : 'CELSIUS'})
            
            return send_response(apcr.get())


    if namespace == 'Alexa.PowerController' or namespace == 'Alexa.ToogleController':

        print('Richiesta tipo PowerController  -----')
        # Note: This sample always returns a success response for either a request to TurnOff or TurnOn
        endpoint_id = request['directive']['endpoint']['endpointId']

        power_state_value = 'OFF' if name == 'TurnOff' else 'ON'

        correlation_token = request['directive']['header']['correlationToken']

        # Check for an error when setting the state
        state_set = set_device_state(endpoint_id=endpoint_id, state='powerState', value=power_state_value)

        if not state_set:

            return AlexaResponse(
                name='ErrorResponse',
                payload={'type': 'ENDPOINT_UNREACHABLE', 'message': 'Unable to reach endpoint database.'}).get()

        apcr = AlexaResponse(correlation_token=correlation_token)
        apcr.add_context_property(namespace='Alexa.PowerController', name='powerState', value=power_state_value)

        return send_response(apcr.get())


# Function to manage send back response to Alexa
def send_response(response):
    # TODO Validate the response
    print('lambda_handler response -----')
    print(json.dumps(response))
    return response


# Function to change device state
def set_device_state(endpoint_id, state, value):

    if value=='ON':
        sAction = '1'
    elif value=='OFF':
        sAction = '0'
    else:
        sAction = 'X'

    str_command =  endpoint_id + "-" + sAction

    attribute_key = state + 'Value'

    response = aws_dynamodb.update_item(
        TableName='SampleSmartHome',
        Key={'ItemId': {'S': endpoint_id}},
        AttributeUpdates={attribute_key: {'Action': 'PUT', 'Value': {'S': value}}})

    print(response)

    if response['ResponseMetadata']['HTTPStatusCode'] == 200:

        print("SEND TCP COMMAND")
        sendTCPCommand(str_command)
        print("RETURN TRUE")
        
        return True

    else:
        return False

# Function to send command over TCP connection
def sendTCPCommand(str_command):

         # Create a TCP/IP socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        destination_address = os.environ['DESTINATION_ADDRESS']
        destination_port_str = os.environ['DESTINATION_PORT']
        secret_key = os.environ['SECRET_KEY']

        print("Connect to " + destination_address + " on TCP port " + destination_port_str)

        destination_port = int(destination_port_str)

        # Connect the socket to the port where the server is listening
        server_address = (destination_address, destination_port)

        sock.connect(server_address)

        str_command = secret_key + "\n" + str_command + "\r\n\r\n";

        print("Command to Send:" + str_command)

        try:

            # Send data
            sock.send(str_command.encode())

            # Look for the response
            #amount_received = 0
            #amount_expected = len(message)

            #while amount_received < amount_expected:
            #    data = sock.recv(16)
            #    amount_received += len(data)

            #print >>sys.stderr, 'received "%s"' % data


        finally:

            print("Close socket")
            sock.close()


        return True
