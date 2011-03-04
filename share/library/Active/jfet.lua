-- Symbol names
local names_n =
{
	en = "N-channel JFET transistor",
	cs = "Tranzistor JFET s kanálem N",
	sk = "Tranzistor JFET s kanálom N"
}

local names_p =
{
	en = "P-channel JFET transistor",
	cs = "Tranzistor JFET s kanálem P",
	sk = "Tranzistor JFET s kanálom P"
}

-- Render area in base units (X1, Y1, X2, Y2)
local area = {-2, -1.5, 2, 1.5}

-- Terminal points
local terminals_n = {{-2, 1}, {2, 1}, {2, -1}}
local terminals_p = {{-2, -1}, {2, 1}, {2, -1}}

-- Rendering
local render = function (cr)
	-- The terminals
	cr.move_to (0, 1)
	cr.line_to (2, 1)

	cr.move_to (0, -1)
	cr.line_to (2, -1)

	-- The ohmic connection
	cr.move_to (0, -1.5)
	cr.line_to (0, 1.5)

	cr.stroke ()
end

local render_n = function (cr)
	render (cr)

	-- The left-side terminal
	cr.move_to (-2, 1)
	cr.line_to (0, 1)

	-- The arrow
	cr.move_to (-1, 0.6)
	cr.line_to (-0.5, 1)
	cr.line_to (-1, 1.4)

	cr.stroke ()
end

local render_p = function (cr)
	render (cr)

	-- The left-side terminal
	cr.move_to (-2, -1)
	cr.line_to (0, -1)

	-- The arrow
	cr.move_to (-0.4, -0.6)
	cr.line_to (-1, -1)
	cr.line_to (-0.4, -1.4)

	cr.stroke ()
end

-- Register the symbols
logdiag.register ("JFET-N", names_n, area, terminals_n, render_n)
logdiag.register ("JFET-P", names_p, area, terminals_p, render_p)


