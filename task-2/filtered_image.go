package main

import (
	"image"
)

type FilteredImage struct {
	OriginalImage image.Image
	FilteredImage image.Image
	Filters       []Filter
}

func NewFilteredImage(img image.Image) *FilteredImage {
	return &FilteredImage{
		OriginalImage: img,
		FilteredImage: img,
		Filters:       []Filter{},
	}
}

func (fi *FilteredImage) ApplyFilters() {
	img := fi.OriginalImage
	for _, filter := range fi.Filters {
		img = filter.Apply(img)
	}
	fi.FilteredImage = img
}

func (fi *FilteredImage) AddFilter(filter Filter) {
	fi.Filters = append(fi.Filters, filter)
	fi.ApplyFilters()
}

func (fi *FilteredImage) ResetFilters() {
	fi.Filters = []Filter{}
	fi.FilteredImage = fi.OriginalImage
}
