FQBN :=  STMicroelectronics:stm32:GenH5
PNUM := pnum=GENERIC_H503CBUX
UPLOAD_METHOD := upload_method=dfuMethod
CDC := usb=CDCgen



build:
	arduino-cli compile \
		--fqbn $(FQBN) \
		--board-options $(PNUM),$(CDC) \

upload:
	arduino-cli upload \
		--fqbn $(FQBN) \
		--board-options $(PNUM),$(UPLOAD_METHOD),$(CDC) \

.PHONY: all build upload
