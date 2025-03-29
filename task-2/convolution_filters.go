package main

import (
	"errors"
	"fmt"
	"image"
	"image/color"
	"io"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"sync"
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
	ySize := len(f.Kernel)
	xSize := len(f.Kernel[0])
	var wg sync.WaitGroup

	for y := bounds.Min.Y; y < bounds.Max.Y; y++ {
		wg.Add(1)
		go func(y int) {
			defer wg.Done()
			for x := bounds.Min.X; x < bounds.Max.X; x++ {
				var rSum, gSum, bSum float64
				for ky := range ySize {
					for kx := range xSize {
						ix := clampToEdge(x+kx-f.AnchorX, bounds.Min.X, bounds.Max.X-1)
						iy := clampToEdge(y+ky-f.AnchorY, bounds.Min.Y, bounds.Max.Y-1)
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
		}(y)
	}

	wg.Wait()
	return filtered
}

func clampToEdge(value, min, max int) int {
	if value < min {
		return min
	} else if value > max {
		return max
	}
	return value
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
	sizes := strings.Split(lines[0], " ")
	if len(sizes) != 2 {
		return nil, errors.New("invalid matrix size format")
	}
	xSize, err := strconv.Atoi(sizes[0])
	if err != nil {
		return nil, fmt.Errorf("invalid matrix size: %w", err)
	}
	ySize, err := strconv.Atoi(sizes[1])
	if err != nil {
		return nil, fmt.Errorf("invalid matrix size: %w", err)
	}
	if xSize%2 == 0 {
		return nil, errors.New("matrix size must be odd")
	}
	if ySize%2 == 0 {
		return nil, errors.New("matrix size must be odd")
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
	kernel := make([][]float64, ySize)
	for i := range ySize {
		kernel[i] = make([]float64, xSize)
		values := strings.Fields(lines[5+i])
		if len(values) != xSize {
			return nil, errors.New("invalid kernel row length")
		}
		for j, value := range values {
			kernel[i][j], err = strconv.ParseFloat(value, 64)
			if err != nil {
				return nil, fmt.Errorf("invalid kernel value: %w", err)
			}
		}
	}

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
	ySize := len(filter.Kernel)
	xSize := len(filter.Kernel[0])
	builder.WriteString(fmt.Sprintf("%d %d\n", xSize, ySize))

	// Write offset
	builder.WriteString(fmt.Sprintf("%f\n", filter.Offset))
	builder.WriteString(fmt.Sprintf("%f\n", filter.Divisor))

	// Write anchor X and Y
	builder.WriteString(fmt.Sprintf("%d\n", filter.AnchorX))
	builder.WriteString(fmt.Sprintf("%d\n", filter.AnchorY))

	// Write kernel
	for i := range ySize {
		for j := range xSize {
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
