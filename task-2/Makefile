# Define the Go compiler
GOCMD = go
GOBUILD = $(GOCMD) build
GOCLEAN = $(GOCMD) clean
GOTEST = $(GOCMD) test
GOGET = $(GOCMD) get
BINARY_NAME = main.o

# All source files
SOURCES = main.go convolution_editor.go convolution_filters.go filter.go filtered_image.go morphological_filters.go

# Default target to build the binary
all: build

# Build the binary with optimizations
build:
	$(GOBUILD) -ldflags="-s -w" -o $(BINARY_NAME) $(SOURCES)

# Run the application
run: build
	./$(BINARY_NAME)

# Clean the build files
clean:
	$(GOCLEAN)
	rm -f $(BINARY_NAME)

# Test the application
test:
	$(GOTEST) -v ./...

# Install dependencies
deps:
	$(GOGET) -u ./...

# Format the code
fmt:
	$(GOCMD) fmt ./...

# Lint the code
lint:
	golangci-lint run

.PHONY: all build clean run test deps fmt lint
