-- Symbol names
local names_igfet_n =
{
	en = "N-channel IGFET transistor",
	cs = "Tranzistor IGFET s kanálem N"
}

local names_igfet_p =
{
	en = "P-channel IGFET transistor",
	cs = "Tranzistor IGFET s kanálem P"
}

-- Render area in base units (X1, Y1, X2, Y2)
local area = {-2, -1.5, 2, 1.5}

-- Terminal points
local terminals = {{-2, 1}, {2, 1}, {2, 0}, {2, -1}}

-- Rendering
local render = function (cr)
	-- The terminals
	cr.move_to (-2, 1)
	cr.line_to (-0.3, 1)
	cr.line_to (-0.3, -1)

	cr.move_to (0, 1)
	cr.line_to (2, 1)

	cr.move_to (0, 0)
	cr.line_to (2, 0)

	cr.move_to (0, -1)
	cr.line_to (2, -1)

	-- Source, gate, drain
	cr.move_to (0, -1.5)
	cr.line_to (0, -0.5)

	cr.move_to (0, -0.3)
	cr.line_to (0, 0.3)

	cr.move_to (0, 0.5)
	cr.line_to (0, 1.5)

	cr.stroke ()
end

local render_igfet_n = function (cr)
	render (cr)

	cr.move_to (0.9, -0.4)
	cr.line_to (0.4, 0)
	cr.line_to (0.9, 0.4)

	cr.stroke ()
end

local render_igfet_p = function (cr)
	render (cr)

	cr.move_to (0.4, -0.4)
	cr.line_to (0.9, 0)
	cr.line_to (0.4, 0.4)

	cr.stroke ()
end

-- Register the symbols
logdiag.register ("IGFET-N", names_igfet_n, area, terminals, render_igfet_n)
logdiag.register ("IGFET-P", names_igfet_p, area, terminals, render_igfet_p)


