# ds3231-idf
Popular RTC  module ds3231 esp-idf component

There are many esp-idf library for the ds3231 RTC module. The popular one is by the UNCLE-RUS(https://github.com/UncleRus/esp-idf-lib). The problem is it uses the i2c dev as a mutex to for the i2c bus. When i2c bus uses other modules which are not available on the uncle rus library, this created the headache to include the i2c dev to incorporate into the project. To simplify this, this library has been created. This libray is the bare minimum needed to run the module. User has to implement their own implementation for guarding the i2c bus.
It inclues only 2 funciton set time and get time. more function can be added on the demand.
