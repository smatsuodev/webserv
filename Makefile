all:

format:
	clang-format -i $$(git ls-files '*.cpp' '*.hpp')