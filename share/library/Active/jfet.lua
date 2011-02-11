-- Symbol names
local names_jfet_n =
{
	en = "N-channel JFET transistor",
	cs = "Tranzistor JFET s kanálem N"
}

local names_jfet_p =
{
	en = "P-channel JFET transistor",
	cs = "Tranzistor JFET s kanálem P"
}

-- Render area in base units (X1, Y1, X2, Y2)
local area = {-2, -1.5, 2, 1.5}

-- Terminal points
local terminals = {{-2, 1}, {2, 1}, {2, -1}}

-- Rendering
local render = function (cr)
	-- The terminals
	cr.move_to (-2, 1)
	cr.line_to (0, 1)

	cr.move_to (0, 1)
	cr.line_to (2, 1)

	cr.move_to (0, -1)
	cr.line_to (2, -1)

	-- The ohmic connection
	cr.move_to (0, -1.5)
	cr.line_to (0, 1.5)

	cr.stroke ()
end

local render_jfet_n = function (cr)
	render (cr)

	cr.move_to (-1, 0.6)
	cr.line_to (-0.5, 1)
	cr.line_to (-1, 1.4)

	cr.stroke ()
end

local render_jfet_p = function (cr)
	render (cr)

	cr.move_to (-0.4, 0.6)
	cr.line_to (-1, 1)
	cr.line_to (-0.4, 1.4)

	cr.stroke ()
end

-- Register the symbols
logdiag.register ("JFET-N", names_jfet_n, area, terminals, render_jfet_n)
logdiag.register ("JFET-P", names_jfet_p, area, terminals, render_jfet_p)


