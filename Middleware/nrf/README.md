The NRF SDK is not included on the repo because of large file size.

Please put NRF SDK folder inside the Middleware/nrf.
Placing the NRF SDK folder to other directory will cause build errors
because the include paths on the SES project are based on the folder 
structure of the repo.

Structure

Middleware
|---nrf
|-----nrf5_sdk_xxxx
|--------components
|--------config
|--------documentation
|--------........