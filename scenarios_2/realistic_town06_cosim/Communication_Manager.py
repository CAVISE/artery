import zmq
import logging
import os
import json
import shutil
from google.protobuf.json_format import MessageToJson
import sys
from google.protobuf.json_format import Parse, MessageToJson
sys.path.append("/artery/scenarios/realistic_town06_cosim/Protobufs_structure")
from Protobufs_structure.Artery_message_structure_pb2 import Artery_message
from Protobufs_structure.OpenCDA_message_structure_pb2 import OpenCDA_message


class CommunicationManagerBuilder:
    """
    Builder class for creating a CommunicationManager with optional configurations.
    """


    def __init__(self, address):
        """
        Initialize the builder with the specified address.

        Parameters:
        - address (str): The address for the CommunicationManager.
        """
        self.communication_manager = CommunicationManager(address)


    def with_error_handler(self):
        """
        Configure the CommunicationManager to use its default error handler.

        Returns:
        - self: The builder instance for method chaining.
        """
        self.communication_manager.error_handler = self.communication_manager.default_error_handler
        return self


    def with_logger(self, logger_creator):
        """
        Configure the CommunicationManager with a logger using the provided logger creator function.

        Parameters:
        - logger_creator (callable): A function that creates a logger.

        Returns:
        - self: The builder instance for method chaining.
        """
        self.communication_manager.logger = CommunicationManager.default_logger()
        return self


    def build(self):
        """
        Build and return the configured CommunicationManager instance.

        Returns:
        - CommunicationManager: The configured CommunicationManager instance.
        """
        return self.communication_manager


class CommunicationManager:
    """
    Main class for managing communication using ZeroMQ.
    """

    def __init__(self, address):
        """
        Initialize the CommunicationManager with the specified address.

        Parameters:
        - address (str): The address for the CommunicationManager.
        """
        self.address = address
        self.context = zmq.Context()
        self.error_handler = self.default_error_handler
        self.logger = None
        self.socket = None


    def create_socket(self, socket_type, start_func):
        """
        Create a ZeroMQ socket and apply the specified start function.

        Parameters:
        - socket_type (int): The type of the ZeroMQ socket.
        - start_func (str): Either 'bind' or 'connect' to start the socket.

        Raises:
        - zmq.ZMQError: If there is an error during socket creation or configuration.
        """
        try:
            self.socket = self.context.socket(socket_type)
            if start_func == 'bind':
                self.socket.bind(self.address)
                if self.logger: self.logger.info(f"OK: Socket is open in {start_func} mode at {self.address}")
            if start_func == 'connect':
                self.socket.connect(self.address)
                if self.logger: self.logger.info(f"OK: Socket is open in {start_func} mode at {self.address}")
        except zmq.ZMQError as e:
            if self.error_handler:
                self.error_handler(e)


    def send_message(self, message):
        """
        Send a message using the configured socket.

        Parameters:
        - message (protobuf): The message to be sent.

        Raises:
        - zmq.ZMQError: If there is an error during message sending.
        """
        try:
            self.socket.send(message)
            if self.logger: self.logger.info(f"OK: Message sent to {self.address}")
        except zmq.ZMQError as e:
            if self.error_handler:
                self.error_handler(e)


    def receive_message(self):
        """
        Receive a message using the configured socket.

        Returns:
        - protobuf: The received message.

        Raises:
        - zmq.ZMQError: If there is an error during message reception.
        """
        try:
            if self.logger: self.logger.info(f"OK: Message received from {self.address}")
            received_data = self.socket.recv()
            return received_data
        except zmq.ZMQError as e:
            if self.error_handler:
                self.error_handler(e)


    def close_socket(self):
        """
        Close the socket if it is open.
        """
        if self.socket:
            self.socket.close()


    def close_context(self):
        """
        Terminate the ZeroMQ context if it is active.
        """
        if self.context:
            self.context.term()


    def default_error_handler(self, error):
        """
        Default error handler method. Closes the socket and terminates the context on error.

        Parameters:
        - error (zmq.ZMQError): The ZeroMQ error that occurred.
        """
        self.close_socket()
        self.close_context()
        if self.logger: self.logger.error(f"ERROR: {error}")


    @classmethod
    def default_logger(cls, log_file_name='communication_logs.log'):
        """
        Create and configure a default logger.

        Parameters:
        - log_file_path (str): The path to the log file.

        Returns:
        - logging.Logger: The configured logger instance.
        """
        logs_folder_path = os.path.join(os.path.dirname(__file__), 'Logs')
        log_file_path = os.path.join(logs_folder_path, log_file_name)

        logging.basicConfig(level=logging.INFO,
                            format='%(asctime)s - %(levelname)s - %(message)s',
                            datefmt='%Y-%m-%d %H:%M:%S')
        file_handler = logging.FileHandler(log_file_path)
        file_handler.setLevel(logging.INFO)
        file_formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
        file_handler.setFormatter(file_formatter)

        logger = logging.getLogger('communication_logger')
        logger.addHandler(file_handler)

        return logger



def read_and_merge_messages(directory):
    merged_message = Artery_message()

    for filename in os.listdir(directory):
        if filename.endswith(".json"):
            filepath = os.path.join(directory, filename)
            temp_filepath = filepath + '.copy'
            shutil.copy(filepath, temp_filepath)
            with open(temp_filepath, 'r') as json_file:
                json_data = json.load(json_file)
                artery_message = Artery_message()
                Parse(json.dumps(json_data), artery_message)
                merged_message.MergeFrom(artery_message)
            os.remove(temp_filepath)

    return merged_message

if __name__ == "__main__":
    address = "tcp://*:7777"
    cm1 = (
        CommunicationManagerBuilder(address)
        .with_error_handler()
        .with_logger(CommunicationManager.default_logger)
        .build()
    )
    cm1.create_socket(zmq.PAIR, 'bind')
    while True:
        mes_in = cm1.receive_message()

        with open("Simulators_messages/OpenCDA_message.proto", 'wb') as f:
            f.write(mes_in)
            opencda_message = OpenCDA_message()
            opencda_message.ParseFromString(mes_in)
            open_cda_json_output = MessageToJson(opencda_message, including_default_value_fields=True, preserving_proto_field_name=True)
            with open("Simulators_messages/OpenCDA_message.json", "w") as json_file:
                json_file.write(open_cda_json_output)


            output_message = Artery_message()
            
            output_message = read_and_merge_messages("Messages_JSON/")

            
            json_output = MessageToJson(output_message, including_default_value_fields=True, preserving_proto_field_name=True)
            with open("Simulators_messages/Artery_message.proto", 'wb') as f:
                f.write(output_message.SerializeToString())
            with open("Simulators_messages/Artery_message.json", "w") as json_file:
                json_file.write(json_output)

        serialized_output_message = output_message.SerializeToString()
        cm1.send_message(serialized_output_message)

