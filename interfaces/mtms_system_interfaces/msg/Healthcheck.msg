# Contains information about the status of a system or component.

# Status values
uint8 READY = 0
uint8 NOT_READY = 1
uint8 DISABLED = 2
uint8 ERROR = 3

# Indicates the status of the component.
uint8 status

# A human-readable status message for the UI.
string status_message

# A human-readable message for the UI to indicate an action that the user can take.
string actionable_message
