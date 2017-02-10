default: unicode

.PHONY: unicode

unicode: ../include/unicode_data.hpp

../include/unicode_data.hpp : unicode.py unicode_data_template.hpp
	./unicode.py > ../include/unicode_data.hpp
