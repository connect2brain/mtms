# When received, neuronavigation will create a new marker with ElectricTarget displacements
# and brain response amplitude (MEP for example or TEP in the future).
# This could be a service, but as neuronavigation is not always available, using a
# topic allows the message to be sent without waiting for a response.
targeting_interfaces/ElectricTarget[] targets
float64 brain_response_amplitude
