package main

import (
	"errors"
	"fmt"
	"image"
	"image/color"
	"io"
	"log"
	"os"
	"path/filepath"
	"strconv"
	"strings"
)

type ConvolutionFilter struct {
	Name    string      `json:"name"`
	Kernel  [][]float64 `json:"kernel"`
	Divisor float64     `json:"divisor"`
	Offset  float64     `json:"offset"`
	AnchorX int         `json:"anchor_x"`
	AnchorY int         `json:"anchor_y"`
}

func (f ConvolutionFilter) Apply(img image.Image) image.Image {
	bounds := img.Bounds()
	filtered := image.NewRGBA(bounds)
	kernelSize := len(f.Kernel)
	offset := kernelSize / 2

	for y := bounds.Min.Y + offset; y < bounds.Max.Y-offset; y++ {
		for x := bounds.Min.X + offset; x < bounds.Max.X-offset; x++ {
			var rSum, gSum, bSum float64
			for ky := 0; ky < kernelSize; ky++ {
				for kx := 0; kx < kernelSize; kx++ {
					ix := x + kx - f.AnchorX
					iy := y + ky - f.AnchorY
					r, g, b, _ := img.At(ix, iy).RGBA()

					// Normalize to [0,1] range
					normalizedR := normalizeColor(r)
					normalizedG := normalizeColor(g)
					normalizedB := normalizeColor(b)

					// Apply kernel weight
					weight := f.Kernel[ky][kx]
					rSum += weight * normalizedR
					gSum += weight * normalizedG
					bSum += weight * normalizedB
				}
			}
			// Scale back to 0-255, apply divisor and offset
			color := color.RGBA{
				R: clamp((rSum * 255 / f.Divisor) + f.Offset),
				G: clamp((gSum * 255 / f.Divisor) + f.Offset),
				B: clamp((bSum * 255 / f.Divisor) + f.Offset),
				A: 255, // Keep alpha fully opaque
			}
			filtered.Set(x, y, color)
		}
	}
	log.Printf("Applied convolution filter")
	return filtered
}

func LoadConvolutionFilters(folder string) ([]ConvolutionFilter, error) {
	var filters []ConvolutionFilter
	files, err := os.ReadDir(folder)
	if err != nil {
		return nil, err
	}

	for _, file := range files {
		if filepath.Ext(file.Name()) == ".filter" {
			file, err := os.Open(filepath.Join(folder, file.Name()))
			if err != nil {
				return nil, err
			}
			defer file.Close()
			filter, err := LoadConvolutionFilter(file)
			if err != nil {
				return nil, err
			}
			filter.Name = strings.TrimSuffix(file.Name(), ".filter")
			filters = append(filters, *filter)
		}
	}
	return filters, nil
}

func LoadConvolutionFilter(file io.ReadCloser) (*ConvolutionFilter, error) {
	data, err := io.ReadAll(file)
	if err != nil {
		return nil, err
	}
	lines := strings.Split(string(data), "\n")
	if len(lines) < 4 {
		return nil, errors.New("invalid filter file format")
	}

	// Read matrix size
	size, err := strconv.Atoi(lines[0])
	if err != nil {
		return nil, fmt.Errorf("invalid matrix size: %w", err)
	}

	// Read offset
	offset, err := strconv.ParseFloat(lines[1], 64)
	if err != nil {
		return nil, fmt.Errorf("invalid offset: %w", err)
	}

	divisor, err := strconv.ParseFloat(lines[2], 64)
	if err != nil {
		return nil, fmt.Errorf("invalid divisor: %w", err)
	}

	// Read anchor X and Y
	anchorX, err := strconv.Atoi(lines[3])
	if err != nil {
		return nil, fmt.Errorf("invalid anchor X: %w", err)
	}
	anchorY, err := strconv.Atoi(lines[4])
	if err != nil {
		return nil, fmt.Errorf("invalid anchor Y: %w", err)
	}

	// Read kernel
	kernel := make([][]float64, size)
	for i := 0; i < size; i++ {
		kernel[i] = make([]float64, size)
		values := strings.Fields(lines[5+i])
		if len(values) != size {
			return nil, errors.New("invalid kernel row length")
		}
		for j, value := range values {
			kernel[i][j], err = strconv.ParseFloat(value, 64)
			if err != nil {
				return nil, fmt.Errorf("invalid kernel value: %w", err)
			}
		}
	}
	log.Printf("Kernel: %v", kernel)

	filter := &ConvolutionFilter{
		Kernel:  kernel,
		Divisor: divisor,
		Offset:  offset,
		AnchorX: anchorX,
		AnchorY: anchorY,
	}
	return filter, nil
}

func SaveConvolutionFilter(filter ConvolutionFilter, file io.WriteCloser) error {
	var builder strings.Builder

	// Write matrix size
	size := len(filter.Kernel)
	builder.WriteString(fmt.Sprintf("%d\n", size))

	// Write offset
	builder.WriteString(fmt.Sprintf("%f\n", filter.Offset))
	builder.WriteString(fmt.Sprintf("%f\n", filter.Divisor))

	// Write anchor X and Y
	builder.WriteString(fmt.Sprintf("%d\n", filter.AnchorX))
	builder.WriteString(fmt.Sprintf("%d\n", filter.AnchorY))

	// Write kernel
	for i := 0; i < size; i++ {
		for j := 0; j < size; j++ {
			builder.WriteString(fmt.Sprintf("%f ", filter.Kernel[i][j]))
		}
		builder.WriteString("\n")
	}

	_, err := file.Write([]byte(builder.String()))
	return err
}

func normalizeColor(value uint32) float64 {
	return float64(value) / 65535.0
}

func clamp(value float64) uint8 {
	if value > 255 {
		return 255
	} else if value < 0 {
		return 0
	}
	return uint8(value)
}
