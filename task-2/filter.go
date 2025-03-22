package main

import (
	"image"
	"image/color"
	"math"
)

type Filter interface {
	Apply(img image.Image) image.Image
}

type InversionFilter struct{}

func (f InversionFilter) Apply(img image.Image) image.Image {
	bounds := img.Bounds()
	inverted := image.NewRGBA(bounds)
	for y := bounds.Min.Y; y < bounds.Max.Y; y++ {
		for x := bounds.Min.X; x < bounds.Max.X; x++ {
			r, g, b, a := img.At(x, y).RGBA()
			inverted.Set(x, y, color.RGBA{
				R: uint8(255 - r/257),
				G: uint8(255 - g/257),
				B: uint8(255 - b/257),
				A: uint8(a / 257),
			})
		}
	}
	return inverted
}

type BrightnessFilter struct {
	Factor float64
}

func (f BrightnessFilter) Apply(img image.Image) image.Image {
	bounds := img.Bounds()
	brightened := image.NewRGBA(bounds)
	for y := bounds.Min.Y; y < bounds.Max.Y; y++ {
		for x := bounds.Min.X; x < bounds.Max.X; x++ {
			r, g, b, a := img.At(x, y).RGBA()

			// Normalize RGB to 0-1 range
			normalizedR := normalizeColor(r)
			normalizedG := normalizeColor(g)
			normalizedB := normalizeColor(b)

			// Apply brightness factor
			brightened.Set(x, y, color.RGBA{
				R: clamp(normalizedR * f.Factor * 255),
				G: clamp(normalizedG * f.Factor * 255),
				B: clamp(normalizedB * f.Factor * 255),
				A: uint8(a / 257),
			})
		}
	}
	return brightened
}

type ContrastFilter struct {
	Factor float64
}

func (f ContrastFilter) Apply(img image.Image) image.Image {
	bounds := img.Bounds()
	contrastEnhanced := image.NewRGBA(bounds)

	for y := bounds.Min.Y; y < bounds.Max.Y; y++ {
		for x := bounds.Min.X; x < bounds.Max.X; x++ {
			// Get the RGBA values as uint32
			r, g, b, a := img.At(x, y).RGBA()

			// Normalize RGB to 0-1 range
			normalizedR := normalizeColor(r)
			normalizedG := normalizeColor(g)
			normalizedB := normalizeColor(b)

			// Apply middle-threshold contrast adjustment
			r8 := f.applyMiddleShift(normalizedR, contrastFactor)
			g8 := f.applyMiddleShift(normalizedG, contrastFactor)
			b8 := f.applyMiddleShift(normalizedB, contrastFactor)

			// Scale back to 0-255
			contrastEnhanced.Set(x, y, color.RGBA{
				R: clamp(r8 * 255),
				G: clamp(g8 * 255),
				B: clamp(b8 * 255),
				A: uint8(a / 257), // Alpha remains the same
			})
		}
	}

	return contrastEnhanced
}

// Helper function to apply middle-threshold contrast based on the pixel value
func (f ContrastFilter) applyMiddleShift(value float64, contrastFactor float64) float64 {
	// Calculate the difference from the middle value (0.5)
	diff := value - 0.5

	// Apply contrast adjustment based on the difference from the middle
	newValue := value + contrastFactor*diff

	// Clamp new value to 0-1 range
	if newValue > 1 {
		newValue = 1
	} else if newValue < 0 {
		newValue = 0
	}

	return newValue
}

type GammaFilter struct {
	Gamma float64
}

func (f GammaFilter) Apply(img image.Image) image.Image {
	bounds := img.Bounds()
	gammaCorrected := image.NewRGBA(bounds)
	for y := bounds.Min.Y; y < bounds.Max.Y; y++ {
		for x := bounds.Min.X; x < bounds.Max.X; x++ {
			r, g, b, a := img.At(x, y).RGBA()
			gammaCorrected.Set(x, y, color.RGBA{
				R: clamp(math.Pow(normalizeColor(r), f.Gamma) * 255),
				G: clamp(math.Pow(normalizeColor(g), f.Gamma) * 255),
				B: clamp(math.Pow(normalizeColor(b), f.Gamma) * 255),
				A: uint8(a / 257),
			})
		}
	}
	return gammaCorrected
}
