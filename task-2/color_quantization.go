package main

import (
	"image"
	"image/color"
)

type UniformQuantization struct {
	KR uint32
	KG uint32
	KB uint32
}

// https://muthu.co/reduce-the-number-of-colors-of-an-image-using-uniform-quantization/
func (u UniformQuantization) Apply(img image.Image) image.Image {
	bounds := img.Bounds()
	quantized := image.NewRGBA(bounds)

	u.KR = clampRegions(u.KR)
	u.KG = clampRegions(u.KG)
	u.KB = clampRegions(u.KB)

	stepR := 256 / u.KR
	stepG := 256 / u.KG
	stepB := 256 / u.KB

	sumR := make([]int, u.KR)
	sumG := make([]int, u.KG)
	sumB := make([]int, u.KB)
	countR := make([]int, u.KR)
	countG := make([]int, u.KG)
	countB := make([]int, u.KB)

	for y := bounds.Min.Y; y < bounds.Max.Y; y++ {
		for x := bounds.Min.X; x < bounds.Max.X; x++ {
			r, g, b, _ := img.At(x, y).RGBA()

			regionR := clampIndex(r/257/stepR, u.KR)
			regionG := clampIndex(g/257/stepG, u.KG)
			regionB := clampIndex(b/257/stepB, u.KB)

			sumR[regionR] += int(r / 257)
			sumG[regionG] += int(g / 257)
			sumB[regionB] += int(b / 257)

			countR[regionR]++
			countG[regionG]++
			countB[regionB]++
		}
	}

	repR := make([]int, u.KR)
	repG := make([]int, u.KG)
	repB := make([]int, u.KB)

	for i := uint32(0); i < u.KR; i++ {
		if countR[i] > 0 {
			repR[i] = sumR[i] / countR[i]
		}
	}
	for i := uint32(0); i < u.KG; i++ {
		if countG[i] > 0 {
			repG[i] = sumG[i] / countG[i]
		}
	}
	for i := uint32(0); i < u.KB; i++ {
		if countB[i] > 0 {
			repB[i] = sumB[i] / countB[i]
		}
	}

	for y := bounds.Min.Y; y < bounds.Max.Y; y++ {
		for x := bounds.Min.X; x < bounds.Max.X; x++ {
			r, g, b, a := img.At(x, y).RGBA()

			regionR := clampIndex(r/257/stepR, u.KR)
			regionG := clampIndex(g/257/stepG, u.KG)
			regionB := clampIndex(b/257/stepB, u.KB)

			quantized.Set(x, y, color.RGBA{
				R: uint8(repR[regionR]),
				G: uint8(repG[regionG]),
				B: uint8(repB[regionB]),
				A: uint8(a / 257),
			})
		}
	}

	return quantized
}

func clampRegions(k uint32) uint32 {
	if k > 256 {
		return 256
	}
	if k < 1 {
		return 1
	}
	return k
}

func clampIndex(index uint32, max uint32) uint32 {
	if index >= max {
		return max - 1
	}
	return index
}
