-- Symbol names
local names_n =
{
	en = "N-channel IGFET transistor",
	cs = "Tranzistor IGFET s kanálem N",
	sk = "Tranzistor IGFET s kanálom N",
	pl = "Tranzystor IGFET z kanałem N",
	de = "N-Kanal IGFET Transistor"
}

local names_p =
{
	en = "P-channel IGFET transistor",
	cs = "Tranzistor IGFET s kanálem P",
	sk = "Tranzistor IGFET s kanálom P",
	pl = "Tranzystor IGFET z kanałem P",
	de = "P-Kanal IGFET Transistor"
}

-- Render area in base units (X1, Y1, X2, Y2)
local area = {-2, -1.5, 2, 1.5}

-- Terminal points
local terminals_n = {{-2, 1}, {2, 1}, {2, 0}, {2, -1}}
local terminals_p = {{-2, -1}, {2, 1}, {2, 0}, {2, -1}}

-- Rendering
local render = function (cr)
	-- The terminals
	cr.move_to (-0.3, 1)
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

local render_n = function (cr)
	render (cr)

	-- The left-side terminal
	cr.move_to (-2, 1)
	cr.line_to (-0.3, 1)

	-- The arrow
	cr.move_to (0.9, -0.4)
	cr.line_to (0.4, 0)
	cr.line_to (0.9, 0.4)

	cr.stroke ()
end

local render_p = function (cr)
	render (cr)

	-- The left-side terminal
	cr.move_to (-2, -1)
	cr.line_to (-0.3, -1)

	-- The arrow
	cr.move_to (0.4, -0.4)
	cr.line_to (0.9, 0)
	cr.line_to (0.4, 0.4)

	cr.stroke ()
end

-- Register the symbols
logdiag.register ("IGFET-N", names_n, area, terminals_n, render_n)
logdiag.register ("IGFET-P", names_p, area, terminals_p, render_p)


