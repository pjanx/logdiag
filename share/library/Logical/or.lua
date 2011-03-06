-- Symbol name
local names =
{
	en = "OR",
	cs = "OR",
	sk = "OR",
	pl = "OR"
}

-- Render area in base units (X1, Y1, X2, Y2)
local area = {-4, -2, 5, 2}

-- Terminal points
local terminals = {{-4, -1}, {-4, 1}, {5, 0}}

-- Rendering
local render = function (cr)
	-- The main shape
	cr.move_to (-2, -2)
	cr.line_to (0, -2)
	cr.curve_to (2, -2, 3, 0, 3, 0)
	cr.curve_to (3, 0, 2, 2, 0, 2)
	cr.line_to (-2, 2)
	cr.curve_to (-1, 1, -1, -1, -2, -2)
	cr.stroke ()

	-- The terminals
	cr.save ()

	-- Crop the contacts according to
	-- the left side of the main shape
	cr.move_to (-4, 2)
	cr.line_to (-2, 2)
	cr.curve_to (-1, 1, -1, -1, -2, -2)
	cr.line_to (-4, -2)
	cr.close_path ()
	cr.clip ()

	cr.move_to (-4, -1)
	cr.line_to (-1, -1)

	cr.move_to (-4, 1)
	cr.line_to (-1, 1)

	cr.stroke ()
	cr.restore ()

	cr.move_to (3, 0)
	cr.line_to (5, 0)
	cr.stroke ()
end

-- Register the symbol
logdiag.register ("OR", names, area, terminals, render)


